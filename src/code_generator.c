#include "code_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

CodeGenerator* code_generator_create(const char* output_filename, OutputType type) {
    CodeGenerator* gen = malloc(sizeof(CodeGenerator));
    gen->output_file = fopen(output_filename, "w");
    if (!gen->output_file) {
        free(gen);
        return NULL;
    }
    
    gen->output_type = type;
    gen->symbol_table = NULL;
    gen->label_counter = 0;
    gen->temp_counter = 0;
    gen->current_offset = 0;
    gen->current_function = NULL;
    
    return gen;
}

void code_generator_destroy(CodeGenerator* generator) {
    if (generator) {
        if (generator->output_file) {
            fclose(generator->output_file);
        }
        free(generator->current_function);
        free(generator);
    }
}

int generate_code(CodeGenerator* generator, ASTNode* ast, SymbolTable* symbols) {
    if (!generator || !ast) return 0;
    
    generator->symbol_table = symbols;
    
    // Cabeçalho do arquivo gerado
    switch (generator->output_type) {
        case OUTPUT_C:
            emit_comment(generator, "Código C gerado pelo compilador");
            emit_code(generator, "#include <stdio.h>\n");
            emit_code(generator, "#include <stdlib.h>\n\n");
            break;
            
        case OUTPUT_ASSEMBLY:
            emit_comment(generator, "Assembly x86-64 gerado pelo compilador");
            emit_code(generator, ".section .text\n");
            emit_code(generator, ".global _start\n\n");
            break;
            
        case OUTPUT_BYTECODE:
            emit_comment(generator, "Bytecode gerado pelo compilador");
            break;
    }
    
    generate_program(generator, ast);
    return 1;
}

void generate_program(CodeGenerator* gen, ASTNode* node) {
    for (int i = 0; i < node->child_count; i++) {
        ASTNode* child = node->children[i];
        
        switch (child->type) {
            case AST_FUNCTION_DECLARATION:
                generate_function_declaration(gen, child);
                break;
            case AST_VARIABLE_DECLARATION:
                generate_variable_declaration(gen, child);
                break;
            default:
                break;
        }
    }
}

void generate_function_declaration(CodeGenerator* gen, ASTNode* node) {
    const char* func_name = node->data.function_decl.name;
    
    free(gen->current_function);
    gen->current_function = strdup(func_name);
    gen->current_offset = 0;
    
    switch (gen->output_type) {
        case OUTPUT_C:
            emit_code(gen, "%s %s(", 
                     data_type_to_string(node->data.function_decl.return_type),
                     func_name);
            
            // Parâmetros (simplificado)
            emit_code(gen, ") {\n");
            
            // Corpo da função
            if (node->data.function_decl.body) {
                generate_statement(gen, node->data.function_decl.body);
            }
            
            emit_code(gen, "}\n\n");
            break;
            
        case OUTPUT_ASSEMBLY:
            emit_code(gen, "%s:\n", func_name);
            emit_code(gen, "    push %%rbp\n");
            emit_code(gen, "    mov %%rsp, %%rbp\n");
            
            if (node->data.function_decl.body) {
                generate_statement(gen, node->data.function_decl.body);
            }
            
            emit_code(gen, "    pop %%rbp\n");
            emit_code(gen, "    ret\n\n");
            break;
            
        case OUTPUT_BYTECODE:
            emit_code(gen, "FUNC %s\n", func_name);
            if (node->data.function_decl.body) {
                generate_statement(gen, node->data.function_decl.body);
            }
            emit_code(gen, "ENDFUNC\n\n");
            break;
    }
}

void generate_variable_declaration(CodeGenerator* gen, ASTNode* node) {
    const char* var_name = node->data.var_decl.name;
    
    switch (gen->output_type) {
        case OUTPUT_C:
            emit_code(gen, "    %s %s", 
                     data_type_to_string(node->data.var_decl.var_type),
                     var_name);
            
            if (node->data.var_decl.initializer) {
                emit_code(gen, " = ");
                generate_expression(gen, node->data.var_decl.initializer);
            }
            
            emit_code(gen, ";\n");
            break;
            
        case OUTPUT_ASSEMBLY:
            gen->current_offset -= 8;  // Assumindo 8 bytes por variável
            emit_comment(gen, "Declaração de variável");
            emit_code(gen, "    # %s at offset %d\n", var_name, gen->current_offset);
            
            if (node->data.var_decl.initializer) {
                generate_expression(gen, node->data.var_decl.initializer);
                emit_code(gen, "    mov %%rax, %d(%%rbp)\n", gen->current_offset);
            }
            break;
            
        case OUTPUT_BYTECODE:
            emit_code(gen, "DECL %s %s\n", 
                     data_type_to_string(node->data.var_decl.var_type),
                     var_name);
            
            if (node->data.var_decl.initializer) {
                generate_expression(gen, node->data.var_decl.initializer);
                emit_code(gen, "STORE %s\n", var_name);
            }
            break;
    }
}

void generate_statement(CodeGenerator* gen, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_COMPOUND_STATEMENT:
            for (int i = 0; i < node->child_count; i++) {
                generate_statement(gen, node->children[i]);
            }
            break;
            
        case AST_VARIABLE_DECLARATION:
            generate_variable_declaration(gen, node);
            break;
            
        case AST_EXPRESSION_STATEMENT:
            if (node->child_count > 0) {
                generate_expression(gen, node->children[0]);
                if (gen->output_type == OUTPUT_C) {
                    emit_code(gen, ";\n");
                }
            }
            break;
            
        case AST_RETURN_STATEMENT:
            if (gen->output_type == OUTPUT_C) {
                emit_code(gen, "    return");
                if (node->data.return_stmt.expression) {
                    emit_code(gen, " ");
                    generate_expression(gen, node->data.return_stmt.expression);
                }
                emit_code(gen, ";\n");
            }
            break;
            
        case AST_IF_STATEMENT: {
            char* else_label = generate_label(gen, "else");
            char* end_label = generate_label(gen, "endif");
            
            if (gen->output_type == OUTPUT_C) {
                emit_code(gen, "    if (");
                generate_expression(gen, node->data.if_stmt.condition);
                emit_code(gen, ") {\n");
                generate_statement(gen, node->data.if_stmt.then_stmt);
                
                if (node->data.if_stmt.else_stmt) {
                    emit_code(gen, "    } else {\n");
                    generate_statement(gen, node->data.if_stmt.else_stmt);
                }
                emit_code(gen, "    }\n");
            }
            
            free(else_label);
            free(end_label);
            break;
        }
        
        default:
            break;
    }
}

void generate_expression(CodeGenerator* gen, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_IDENTIFIER:
            emit_code(gen, "%s", node->data.identifier.name);
            break;
            
        case AST_NUMBER_LITERAL:
            emit_code(gen, "%s", node->data.literal.value);
            break;
            
        case AST_STRING_LITERAL:
            emit_code(gen, "\"%s\"", node->data.literal.value);
            break;
            
        case AST_BINARY_EXPRESSION:
            if (gen->output_type == OUTPUT_C) {
                emit_code(gen, "(");
                generate_expression(gen, node->data.binary_expr.left);
                
                switch (node->data.binary_expr.operator) {
                    case TOKEN_PLUS: emit_code(gen, " + "); break;
                    case TOKEN_MINUS: emit_code(gen, " - "); break;
                    case TOKEN_MULTIPLY: emit_code(gen, " * "); break;
                    case TOKEN_DIVIDE: emit_code(gen, " / "); break;
                    case TOKEN_EQUAL: emit_code(gen, " == "); break;
                    case TOKEN_NOT_EQUAL: emit_code(gen, " != "); break;
                    case TOKEN_LESS: emit_code(gen, " < "); break;
                    case TOKEN_GREATER: emit_code(gen, " > "); break;
                    default: emit_code(gen, " ? "); break;
                }
                
                generate_expression(gen, node->data.binary_expr.right);
                emit_code(gen, ")");
            }
            break;
            
        case AST_FUNCTION_CALL:
            emit_code(gen, "%s(", node->data.function_call.name);
            for (int i = 0; i < node->child_count; i++) {
                if (i > 0) emit_code(gen, ", ");
                generate_expression(gen, node->children[i]);
            }
            emit_code(gen, ")");
            break;
            
        default:
            break;
    }
}

char* generate_label(CodeGenerator* gen, const char* prefix) {
    char* label = malloc(64);
    snprintf(label, 64, "%s_%d", prefix, gen->label_counter++);
    return label;
}

char* generate_temp_var(CodeGenerator* gen) {
    char* temp = malloc(32);
    snprintf(temp, 32, "_temp_%d", gen->temp_counter++);
    return temp;
}

void emit_code(CodeGenerator* gen, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(gen->output_file, format, args);
    va_end(args);
}

void emit_comment(CodeGenerator* gen, const char* comment) {
    switch (gen->output_type) {
        case OUTPUT_C:
            fprintf(gen->output_file, "/* %s */\n", comment);
            break;
        case OUTPUT_ASSEMBLY:
            fprintf(gen->output_file, "# %s\n", comment);
            break;
        case OUTPUT_BYTECODE:
            fprintf(gen->output_file, "; %s\n", comment);
            break;
    }
}
