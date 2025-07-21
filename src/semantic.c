#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SemanticAnalyzer* semantic_analyzer_create() {
    SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
    analyzer->symbol_table = symbol_table_create();
    analyzer->has_error = 0;
    analyzer->error_message[0] = '\0';
    analyzer->current_function_return_type = TYPE_VOID;
    analyzer->in_loop = 0;
    analyzer->error_count = 0;
    analyzer->warning_count = 0;
    
    // Adicionar funções built-in
    Symbol* printf_sym = symbol_create_function("printf", TYPE_INT, 0, 0);
    symbol_table_insert(analyzer->symbol_table, printf_sym);
    
    Symbol* scanf_sym = symbol_create_function("scanf", TYPE_INT, 0, 0);
    symbol_table_insert(analyzer->symbol_table, scanf_sym);
    
    return analyzer;
}

void semantic_analyzer_destroy(SemanticAnalyzer* analyzer) {
    if (analyzer) {
        symbol_table_destroy(analyzer->symbol_table);
        free(analyzer);
    }
}

void semantic_error(SemanticAnalyzer* analyzer, const char* message, int line, int column) {
    analyzer->has_error = 1;
    analyzer->error_count++;
    snprintf(analyzer->error_message, sizeof(analyzer->error_message),
             "Erro semântico na linha %d, coluna %d: %s", line, column, message);
}

int semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* ast) {
    if (!ast) return 0;
    
    switch (ast->type) {
        case AST_PROGRAM:
            for (int i = 0; i < ast->child_count; i++) {
                analyze_declaration(analyzer, ast->children[i]);
                if (analyzer->has_error) return 0;
            }
            break;
            
        case AST_FUNCTION_DECLARATION:
            analyze_declaration(analyzer, ast);
            break;
            
        case AST_VARIABLE_DECLARATION:
            analyze_declaration(analyzer, ast);
            break;
            
        default:
            analyze_statement(analyzer, ast);
            break;
    }
    
    return !analyzer->has_error;
}

void analyze_declaration(SemanticAnalyzer* analyzer, ASTNode* decl) {
    switch (decl->type) {
        case AST_FUNCTION_DECLARATION: {
            const char* name = decl->data.function_decl.name;
            DataType return_type = decl->data.function_decl.return_type;
            
            // Verificar se função já foi declarada
            Symbol* function_symbol = symbol_create_function(name, return_type, decl->line, decl->column);
            if (!symbol_table_insert(analyzer->symbol_table, function_symbol)) {
                semantic_error(analyzer, "Função já declarada", decl->line, decl->column);
                free(function_symbol->name);
                free(function_symbol);
                return;
            }
            
            // Criar novo escopo para a função
            symbol_table_enter_scope(analyzer->symbol_table, name);
            analyzer->current_function_return_type = return_type;
            
            // Analisar corpo da função
            if (decl->data.function_decl.body) {
                analyze_statement(analyzer, decl->data.function_decl.body);
            }
            
            // Restaurar escopo anterior
            symbol_table_exit_scope(analyzer->symbol_table);
            break;
        }
        
        case AST_VARIABLE_DECLARATION: {
            const char* name = decl->data.var_decl.name;
            DataType type = decl->data.var_decl.var_type;
            
            // Verificar se variável já foi declarada no escopo atual
            Symbol* var_symbol = symbol_create_variable(name, type, decl->line, decl->column);
            if (!symbol_table_insert(analyzer->symbol_table, var_symbol)) {
                semantic_error(analyzer, "Variável já declarada", decl->line, decl->column);
                free(var_symbol->name);
                free(var_symbol);
                return;
            }
            
            // Analisar inicializador se existir
            if (decl->data.var_decl.initializer) {
                DataType init_type = analyze_expression(analyzer, decl->data.var_decl.initializer);
                if (!check_type_compatibility(type, init_type) && init_type != TYPE_VOID) {
                    semantic_error(analyzer, "Tipo incompatível na inicialização", 
                                 decl->line, decl->column);
                }
            }
            break;
        }
        
        default:
            analyze_statement(analyzer, decl);
            break;
    }
}

void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case AST_COMPOUND_STATEMENT: {
            // Criar novo escopo para bloco
            symbol_table_enter_scope(analyzer->symbol_table, "block");
            
            for (int i = 0; i < stmt->child_count; i++) {
                analyze_statement(analyzer, stmt->children[i]);
                if (analyzer->has_error) break;
            }
            
            // Restaurar escopo anterior
            symbol_table_exit_scope(analyzer->symbol_table);
            break;
        }
            
        case AST_EXPRESSION_STATEMENT:
            if (stmt->child_count > 0) {
                analyze_expression(analyzer, stmt->children[0]);
            }
            break;
            
        case AST_IF_STATEMENT: {
            DataType cond_type = analyze_expression(analyzer, stmt->data.if_stmt.condition);
            
            // Condições válidas: qualquer tipo numérico (int, float, char)
            if (cond_type == TYPE_VOID) {
                semantic_error(analyzer, "Condição inválida em if", stmt->line, stmt->column);
            }
            // Aceitar int, float, char como condições válidas
            
            analyze_statement(analyzer, stmt->data.if_stmt.then_stmt);
            if (stmt->data.if_stmt.else_stmt) {
                analyze_statement(analyzer, stmt->data.if_stmt.else_stmt);
            }
            break;
        }
        
        case AST_WHILE_STATEMENT: {
            analyzer->in_loop++;
            DataType cond_type = analyze_expression(analyzer, stmt->data.while_stmt.condition);
            
            // Condições válidas: qualquer tipo numérico
            if (cond_type == TYPE_VOID) {
                semantic_error(analyzer, "Condição inválida em while", stmt->line, stmt->column);
            }
            
            analyze_statement(analyzer, stmt->data.while_stmt.body);
            analyzer->in_loop--;
            break;
        }
        
        case AST_RETURN_STATEMENT: {
            DataType return_type = TYPE_VOID;
            if (stmt->data.return_stmt.expression) {
                return_type = analyze_expression(analyzer, stmt->data.return_stmt.expression);
            }
            
            if (!check_type_compatibility(return_type, analyzer->current_function_return_type)) {
                semantic_error(analyzer, "Tipo de retorno incompatível", 
                             stmt->line, stmt->column);
            }
            break;
        }

        case AST_BREAK_STATEMENT:
            if (analyzer->in_loop <= 0) {
                semantic_error(analyzer, "Comando 'break' fora de um loop", stmt->line, stmt->column);
            }
            break;
        
        case AST_CONTINUE_STATEMENT:
            if (analyzer->in_loop <= 0) {
                semantic_error(analyzer, "Comando 'continue' fora de um loop", stmt->line, stmt->column);
            }
            break;
        
        case AST_VARIABLE_DECLARATION:
            // Tratar declarações dentro de blocos
            analyze_declaration(analyzer, stmt);
            break;
            
        default:
            break;
    }
}

DataType analyze_expression(SemanticAnalyzer* analyzer, ASTNode* expr) {
    if (!expr) return TYPE_VOID;
    
    switch (expr->type) {
        case AST_IDENTIFIER: {
            Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, 
                                               expr->data.identifier.name);
            if (!symbol) {
                semantic_error(analyzer, "Identificador não declarado", 
                             expr->line, expr->column);
                return TYPE_VOID;
            }
            return symbol->type;
        }
        
        case AST_NUMBER_LITERAL: {
            // Verificar se tem ponto decimal na string
            if (expr->data.literal.value && strchr(expr->data.literal.value, '.')) {
                return TYPE_FLOAT;
            }
            return TYPE_INT;
        }
        
        case AST_FLOAT_LITERAL:
            return TYPE_FLOAT;
            
        case AST_STRING_LITERAL:
            return TYPE_CHAR; // Simplificado: string como char*
            
        case AST_CHAR_LITERAL:
            return TYPE_CHAR;
            
        case AST_BINARY_EXPRESSION: {
            DataType left_type = analyze_expression(analyzer, expr->data.binary_expr.left);
            DataType right_type = analyze_expression(analyzer, expr->data.binary_expr.right);
            
            // Se algum dos operandos é void, há erro anterior
            if (left_type == TYPE_VOID || right_type == TYPE_VOID) {
                return TYPE_VOID;
            }
            
            TokenType op = expr->data.binary_expr.operator;
            return get_binary_operation_result_type(left_type, right_type, op);
        }
        
        case AST_UNARY_EXPRESSION: {
            DataType operand_type = analyze_expression(analyzer, expr->data.unary_expr.operand);
            
            // Para operações unárias, o tipo geralmente se mantém
            if (operand_type == TYPE_VOID) {
                return TYPE_VOID;
            }
            
            UnaryOperator op = expr->data.unary_expr.operator;
            switch (op) {
                case UNARY_NOT:
                    return TYPE_INT; // Resultado de ! é sempre int (0 ou 1)
                case UNARY_MINUS:
                case UNARY_PLUS:
                    return operand_type; // Mantém o tipo original
                default:
                    return operand_type;
            }
        }
        
        case AST_ASSIGNMENT_EXPRESSION: {
            DataType left_type = analyze_expression(analyzer, expr->data.binary_expr.left);
            DataType right_type = analyze_expression(analyzer, expr->data.binary_expr.right);
            
            if (left_type != TYPE_VOID && right_type != TYPE_VOID && 
                !check_type_compatibility(left_type, right_type)) {
                semantic_error(analyzer, "Tipos incompatíveis na atribuição", 
                             expr->line, expr->column);
            }
            
            return left_type;
        }
        
        case AST_FUNCTION_CALL: {
            Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, 
                                               expr->data.function_call.name);
            if (!symbol) {
                // Para printf, não dar erro - tratar como built-in
                if (strcmp(expr->data.function_call.name, "printf") == 0) {
                    // Analisar argumentos
                    for (int i = 0; i < expr->child_count; i++) {
                        analyze_expression(analyzer, expr->children[i]);
                    }
                    return TYPE_INT; // printf retorna int
                }
                
                semantic_error(analyzer, "Função não declarada", 
                             expr->line, expr->column);
                return TYPE_VOID;
            }
            
            if (symbol->kind != SYMBOL_FUNCTION) {
                semantic_error(analyzer, "Identificador não é uma função", 
                             expr->line, expr->column);
                return TYPE_VOID;
            }
            
            // Analisar argumentos
            for (int i = 0; i < expr->child_count; i++) {
                analyze_expression(analyzer, expr->children[i]);
            }
            
            return symbol->type;
        }
        
        default:
            return TYPE_INT; // Padrão conservador
    }
}

void semantic_warning(SemanticAnalyzer* analyzer, const char* message, int line, int column) {
    analyzer->warning_count++;
    printf("Aviso semântico na linha %d, coluna %d: %s\n", line, column, message);
}

int check_type_compatibility(DataType type1, DataType type2) {
    if (type1 == type2) return 1;
    
    // Void não é compatível com nada (exceto void)
    if (type1 == TYPE_VOID || type2 == TYPE_VOID) {
        return 0;
    }
    
    // Conversões implícitas permitidas entre tipos numéricos
    if ((type1 == TYPE_INT && type2 == TYPE_FLOAT) ||
        (type1 == TYPE_FLOAT && type2 == TYPE_INT)) {
        return 1;
    }
    
    if ((type1 == TYPE_INT && type2 == TYPE_CHAR) ||
        (type1 == TYPE_CHAR && type2 == TYPE_INT)) {
        return 1;
    }
    
    if ((type1 == TYPE_FLOAT && type2 == TYPE_CHAR) ||
        (type1 == TYPE_CHAR && type2 == TYPE_FLOAT)) {
        return 1;
    }
    
    return 0;
}

DataType get_binary_operation_result_type(DataType left, DataType right, TokenType op) {
    // Para operadores de comparação, sempre retorna int (0 ou 1)
    if (op == TOKEN_EQUAL || op == TOKEN_NOT_EQUAL ||
        op == TOKEN_LESS || op == TOKEN_GREATER ||
        op == TOKEN_LESS_EQUAL || op == TOKEN_GREATER_EQUAL ||
        op == TOKEN_AND || op == TOKEN_OR) {
        return TYPE_INT;
    }
    
    // Para operadores aritméticos, promover para o tipo "maior"
    if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    
    if (left == TYPE_INT || right == TYPE_INT) {
        return TYPE_INT;
    }
    
    // Se ambos são char, resultado é int (promoção automática em C)
    return TYPE_INT;
}