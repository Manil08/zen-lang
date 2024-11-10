#pragma once
#include <unordered_map>
#include "parser.hpp"

using namespace std;

class Generator
{
public:
    explicit inline Generator(NodeProg* prog)
        : m_prog(move(prog)){}

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator* gen;
            TermVisitor (Generator* gen)
                : gen(gen){}

            void operator()(const NodeTermIdent* term_ident) {
                const string& ident_name = term_ident->ident.value.value();
                if(gen->m_vars.find(ident_name) == gen->m_vars.end() ||
                    gen->m_vars[ident_name].stack_loc >= gen->m_stack_size) {
                    gen->throw_exit_failure("Identifier not found : ",ident_name);
                }
                gen->push(gen->pointer_loc(ident_name));
            }
            void operator()(const NodeTermIntLit* term_int_lit) {
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeTermFuncCall* func_call) {
                string name = func_call->ident.value.value();
                vector<NodeExpr*> para = func_call->parameters;

                if(gen->m_func_names.find(name) == gen->m_func_names.end()) {
                    gen->throw_exit_failure("Function not found : ",name);
                }
                if(gen->m_func_names[name]!=para.size()) {
                    cerr << "Invalid parameters transferred : Required " << gen->m_func_names[name] << ", Found " << para.size() << endl;
                    exit(EXIT_FAILURE);
                }

                unordered_map<string,Var> old_m_vars = gen->m_vars;
                size_t old_m_stack_size = gen->m_stack_size;
                for(auto term:para) {
                    gen->gen_expr(term);
                }

                gen->m_output << "    call " << name << "\n";

                for(auto term:para) gen->pop("rbx");
                gen->push("rax");
            }
        };
        TermVisitor visitor(this);
        visit(visitor,term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator* gen;
            BinExprVisitor(Generator* gen)
                : gen(gen){}

            void operator()(const NodeBinExprAdd* bin_expr_add) {
                gen->gen_expr(bin_expr_add->lhs);
                gen->gen_expr(bin_expr_add->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprAnd* bin_expr_and) {
                gen->gen_expr(bin_expr_and->lhs);
                gen->gen_expr(bin_expr_and->rhs);
                gen->pop("rax"); // rhs
                gen->pop("rbx"); // lhs
                gen->m_output << "    and rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprOr* bin_expr_or) {
                gen->gen_expr(bin_expr_or->lhs);
                gen->gen_expr(bin_expr_or->rhs);
                gen->pop("rax"); // rhs
                gen->pop("rbx"); // lhs
                gen->m_output << "    or rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprMult* bin_expr_mult) const{
                gen->gen_expr(bin_expr_mult->lhs);
                gen->gen_expr(bin_expr_mult->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    imul rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprDiv* bin_expr_div) const{
                gen->gen_expr(bin_expr_div->lhs);
                gen->gen_expr(bin_expr_div->rhs);
                gen->pop("rbx"); // divisor
                gen->pop("rax"); // dividend
                gen->m_output << "    idiv rbx\n";
                gen->push("rax");
            }

            void operator()(const NodeBinExprSub* bin_expr_sub) const{
                gen->gen_expr(bin_expr_sub->lhs);
                gen->gen_expr(bin_expr_sub->rhs);
                gen->pop("rax"); // rhs
                gen->pop("rbx"); // lhs
                gen->m_output << "    sub rbx, rax\n";
                gen->push("rbx");
            }

            void operator()(const NodeBinExprRem* bin_expr_rem) const{
                gen->gen_expr(bin_expr_rem->lhs);
                gen->gen_expr(bin_expr_rem->rhs);
                gen->pop("rbx"); // divisor
                gen->pop("rax"); // dividend
                gen->m_output << "    idiv rbx\n";
                gen->push("rdx");
            }

            void operator()(const NodeBinExprPow* bin_expr_pow) const{
                gen->gen_expr(bin_expr_pow->lhs);
                gen->gen_expr(bin_expr_pow->rhs);
                gen->pop("rbx"); // power
                gen->pop("rax"); // lhs
                gen->m_output << "    mov rcx, " << "rbx\n";
                gen->m_output << "    mov rbx, " << "1\n";
                gen->m_output << "top" << gen->global_id << ":\n";
                gen->m_output << "    imul rbx, rax\n";
                gen->m_output << "loop top" << gen->global_id << "\n";
                gen->push("rbx");
                gen->global_id++;
            }

            void operator()(const NodeBinExprEquals* bin_expr_equals) const{
                gen->gen_expr(bin_expr_equals->lhs);
                gen->gen_expr(bin_expr_equals->rhs);
                gen->pop("rax"); // rhs
                gen->pop("rbx"); // lhs
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    jne S1" << gen->global_id << "\n";
                gen->comp_statements();
            }

            void operator()(const NodeBinExprGT* bin_expr_gt) const{
                gen->gen_expr(bin_expr_gt->lhs);
                gen->gen_expr(bin_expr_gt->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    jle S1" << gen->global_id << "\n";
                gen->comp_statements();
            }

            void operator()(const NodeBinExprGTE* bin_expr_gte) const{
                gen->gen_expr(bin_expr_gte->lhs);
                gen->gen_expr(bin_expr_gte->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    jl S1" << gen->global_id << "\n";
                gen->comp_statements();
            }

            void operator()(const NodeBinExprLT* bin_expr_lt) const{
                gen->gen_expr(bin_expr_lt->lhs);
                gen->gen_expr(bin_expr_lt->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    jge S1" << gen->global_id << "\n";
                gen->comp_statements();
            }

            void operator()(const NodeBinExprLTE* bin_expr_lte) const{
                gen->gen_expr(bin_expr_lte->lhs);
                gen->gen_expr(bin_expr_lte->rhs);
                gen->pop("rbx"); // rhs
                gen->pop("rax"); // lhs
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    jg S1" << gen->global_id << "\n";
                gen->comp_statements();
            }

        };
        BinExprVisitor visitor(this);
        visit(visitor,bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor{

            Generator* gen;
            ExprVisitor(Generator* gen)
                : gen(gen){}

            void operator()(const NodeTerm* term) {
                gen->gen_term(term);
            }

            void operator()(const NodeBinExpr* bin_expr) {
                gen->gen_bin_expr(bin_expr);
            }
        };
        ExprVisitor visitor(this);
        visit(visitor,expr->var);
    }

    void gen_ident(const NodeStmtIdent* node_ident) {
        struct IdentVisitor {
            Generator* gen;
            const NodeStmtIdent* node_ident;
            IdentVisitor(Generator* gen, const NodeStmtIdent* node_ident)
                : gen(gen), node_ident(node_ident){}

            void operator()(const NodeExpr* expr) {
                string point = gen->pointer_loc(node_ident->ident.value.value());
                gen->gen_expr(expr);
                gen->pop("rax");
                gen->m_output << "    mov " << point << ", rax\n";
            }

            void operator()(const NodeStmtIdentInc* inc) {
                string point = gen->pointer_loc(node_ident->ident.value.value());
                gen->m_output << "    inc " << point << "\n";
            }

            void operator()(const NodeStmtIdentDec* dec) {
                string point = gen->pointer_loc(node_ident->ident.value.value());
                gen->m_output << "    dec " << point << "\n";
            }
        };

        IdentVisitor visitor(this, node_ident);
        visit(visitor,node_ident->var);

    }

    void gen_scope(const NodeScope* scope) {
        if(!scope) return;
        size_t init_stack_size = m_stack_size;
        for(auto stmt: scope->stmts) {
            gen_stmt(stmt);
            line_ct++;
        }
        while(m_stack_size>init_stack_size) pop("rax");
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {

            Generator* gen;
            StmtVisitor(Generator* gen)
                : gen(gen){}

            void operator()(const NodeStmtExit* stmt_exit) {
                gen->gen_expr(stmt_exit->expr);
                gen->pop("rdi");
                gen->m_output << "    mov rax, 60\n";
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet* stmt_let) {
                const string& variable_name = stmt_let->ident.value.value();
                if(gen->m_vars.find(variable_name)!=gen->m_vars.end()) {
                    gen->throw_exit_failure("Identifier already used: ",variable_name);
                }
                gen->m_vars[variable_name] = Var{.stack_loc = gen->m_stack_size};
                gen->gen_expr(stmt_let->expr);
            }
            void operator()(const NodeStmtIdent* stmt_ident) {
                const string& variable_name = stmt_ident->ident.value.value();
                if(gen->m_vars.find(variable_name)==gen->m_vars.end() ||
                    gen->m_vars[variable_name].stack_loc>=gen->m_stack_size) {
                    gen->throw_exit_failure("Identifier not found: ",variable_name);
                }
                // gen->m_vars[variable_name] = Var{.stack_loc = gen->m_stack_size};
                gen->gen_ident(stmt_ident);
            }
            void operator()(const NodeStmtIf* stmt_if) {
                gen->global_id++;
                size_t id=gen->global_id;
                auto expr = stmt_if->expr;
                gen->gen_expr(expr);
                gen->pop("rax");
                gen->m_output << "    cmp rax, 1\n";
                gen->m_output << "    jne else" << id <<"\n";
                /* if scope */
                gen->gen_scope(stmt_if->stmts);
                gen->m_output << "    jmp end" << id <<"\n";
                gen->m_output << "else" << id << ":\n";
                /* else scope */
                gen->gen_scope(stmt_if->else_stmts);
                gen->m_output << "end" << id << ":\n";
            }

            void operator()(const NodeScope* scope) {
                gen->gen_scope(scope);
            }

            void operator()(const NodeStmtRet* ret) {
                gen->gen_expr(ret->expr);
                gen->pop("rax");
                while(gen->m_stack_size > gen->m_func_cap) gen->pop("rbx");
                gen->m_output << "    ret\n";
            }

            void operator()(const NodeStmtRep* node_rep) {
                size_t id=gen->global_id;
                gen->gen_expr(node_rep->expr);
                gen->pop("rcx");
                gen->m_output << "l" << id << ":\n";
                gen->gen_scope(node_rep->stmts);
                gen->m_output << "loop l" << id << "\n";
                gen->global_id++;
            }

        };

        StmtVisitor visitor(this);
        visit(visitor,stmt->var);

    }

    void gen_funcdec(const NodeStmtFuncDec* function) {
        string func_name = function->ident.value.value();
        vector<Token> parameters = function->parameters;
        auto scope = function->stmts;

        if(m_func_names.find(func_name) != m_func_names.end()) {
            throw_exit_failure("Duplicate function declarations for ",func_name);
        }
        m_func_names[func_name] = parameters.size();
        m_output << func_name << ":\n";

        for(auto parameter: parameters) {
            string para = parameter.value.value();
            m_vars[para]={.stack_loc = m_stack_size};
            m_stack_size++;
        }
        m_stack_size++;
        m_func_cap = m_stack_size;

        gen_scope(scope);

        m_stack_size=0;
        m_vars.clear();
    }

    string gen_prog() {

        m_output << "global _start\n";

        for(const NodeStmtFuncDec* function: m_prog->functions) {
            gen_funcdec(function);
            line_ct++;
        }

        m_output << "_start:\n";

        for(const NodeStmt* stmt: m_prog->stmts) {
            gen_stmt(stmt);
            line_ct++;
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:

    void push(const string& reg) {
        m_output << "    push " << reg <<"\n";
        m_stack_size++;
    }

    void pop(const string& reg) {
        m_output << "    pop " << reg <<"\n";
        m_stack_size--;
    }

    void comp_statements() {
        push("1");
        m_output << "    jmp S2" << global_id << "\n";
        m_output << "S1" << global_id << ":\n";
        push("0");
        m_stack_size--;
        m_output << "S2" << global_id << ":\n";
        global_id++;
    }

    struct Var {
        size_t stack_loc;
    };

    string pointer_loc(const string& ident) {
        size_t loc = (m_vars[ident]).stack_loc;
        string pointer_location = "QWORD [rsp + " + (to_string((m_stack_size - 1 - loc)*8)) + "]";
        return pointer_location;
    }

    void throw_exit_failure(const string& s, const string& ident) const {
        cerr << "Line "<< line_ct << " : " << s << " : " << ident << endl;
        exit(EXIT_FAILURE);
    }

    stringstream m_output;
    const NodeProg* m_prog;
    size_t m_func_cap;
    unordered_map<string,Var> m_vars;
    unordered_map<string,size_t> m_func_names;

    size_t m_stack_size = 0;
    size_t line_ct = 1;
    size_t global_id = 0;
};