#pragma once
#include <cmath>
#include "tokenization.hpp"
#include "arena.hpp"
#include <variant>
using namespace std;

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermFuncCall {
    Token ident;
    vector<NodeExpr*> parameters;
};

struct NodeTerm {
    variant<NodeTermIdent*,NodeTermIntLit*,NodeTermFuncCall*> var;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMult {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprRem {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprPow {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprGT {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprGTE {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprLT {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprLTE {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprEquals {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprAnd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprOr {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    variant<NodeBinExprAdd*,NodeBinExprMult*,NodeBinExprSub*,
    NodeBinExprDiv*,NodeBinExprRem*, NodeBinExprPow*,
    NodeBinExprEquals*, NodeBinExprGT*, NodeBinExprGTE*,
    NodeBinExprLT*, NodeBinExprLTE*, NodeBinExprAnd*,
    NodeBinExprOr*> var;
};

struct NodeExpr {
    variant<NodeTerm*,NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    NodeExpr* expr;
    Token ident;
};

struct NodeStmtIdentInc{};

struct NodeStmtIdentDec{};

struct NodeStmtIdent {
    variant<NodeExpr*,NodeStmtIdentInc*,NodeStmtIdentDec*> var;
    Token ident;
};

struct NodeStmt;

struct NodeScope {
    vector<NodeStmt*> stmts;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* stmts;
    NodeScope* else_stmts;
};

struct NodeStmtRep {
    NodeExpr* expr;
    NodeScope* stmts;
};

struct NodeStmtRet {
    NodeExpr* expr;
};

struct NodeStmtFuncDec {
    Token ident;
    vector<Token> parameters;
    NodeScope* stmts;
};

struct NodeStmt {
    variant<NodeStmtExit*,NodeStmtLet*,NodeStmtIdent*,NodeStmtIf*,NodeScope*,NodeStmtRep*,NodeStmtRet*> var;
};

struct NodeProg {
    vector<NodeStmt*> stmts;
    vector<NodeStmtFuncDec*> functions;
};

class Parser{
public:

    inline explicit Parser(vector<Token> tokens)
        : m_tokens(move(tokens)),
        m_alloc(1024*1024*8) // 4 MB memory allocated
    {}

    optional<NodeTerm*> parse_term() {
        auto* term = m_alloc.alloc<NodeTerm>();
        if(peek().has_value() && peek().value().type==TokenType::int_lit)
        {
            auto* term_int_lit = m_alloc.alloc<NodeTermIntLit>();
            term_int_lit->int_lit=consume();
            term->var=term_int_lit;
            return term;
        }
        else if(peek().has_value() && peek().value().type==TokenType::ident)
        {
            Token ident = consume();
            if(peek().has_value() && peek().value().type == TokenType::open_squ_paren) {
                auto node_func_call = m_alloc.alloc<NodeTermFuncCall>();
                node_func_call->ident = ident;
                if(auto parameters = parse_para()) {
                    node_func_call->parameters = parameters.value();
                }
                term->var = node_func_call;
                return term;
            }
            else {
                auto* term_ident = m_alloc.alloc<NodeTermIdent>();
                term_ident->ident=ident;
                term->var=term_ident;
                return term;
            }
        }
        else return {};
    }

    optional<NodeExpr*> parse_expr(int min_prec)
    {
        if(peek().has_value() && peek().value().type == TokenType::open_paren) {
            consume();
            bracket_Ct++;
        }
        auto term = parse_term();
        if(!term.has_value()) {
            return {};
        }
        auto expr_lhs = m_alloc.alloc<NodeExpr>();
        expr_lhs->var = term.value();

        while(true) {
            if(peek().has_value() && peek().value().type == TokenType::open_paren) {
                consume();
                bracket_Ct++;
            }
            if(peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume();
                bracket_Ct--;
                if(bracket_Ct<0) throw_exit_failure("Invalid parenthesis!");
                break;
            }
            optional<Token> curr_tok = peek();
            auto curr_prec = bin_prec(curr_tok->type);
            if(!curr_tok.has_value() || !curr_prec.has_value() || curr_prec < min_prec) {
                break;
            }
            auto temp_expr = m_alloc.alloc<NodeExpr>();
            auto bin_expr = m_alloc.alloc<NodeBinExpr>();
            curr_tok = consume();
            auto rhs = parse_expr(min_prec + 1);
            if(!rhs.has_value()) {
                throw_exit_failure("Couldn't parse expression!");
            }
            auto bin_expr_add = m_alloc.alloc<NodeBinExprAdd>();
            auto bin_expr_mult = m_alloc.alloc<NodeBinExprMult>();
            auto bin_expr_div = m_alloc.alloc<NodeBinExprDiv>();
            auto bin_expr_sub = m_alloc.alloc<NodeBinExprSub>();
            auto bin_expr_rem = m_alloc.alloc<NodeBinExprRem>();
            auto bin_expr_pow = m_alloc.alloc<NodeBinExprPow>();
            auto bin_expr_equals = m_alloc.alloc<NodeBinExprEquals>();
            auto bin_expr_gt = m_alloc.alloc<NodeBinExprGT>();
            auto bin_expr_gte = m_alloc.alloc<NodeBinExprGTE>();
            auto bin_expr_lt = m_alloc.alloc<NodeBinExprLT>();
            auto bin_expr_lte = m_alloc.alloc<NodeBinExprLTE>();
            auto bin_expr_and = m_alloc.alloc<NodeBinExprAnd>();
            auto bin_expr_or = m_alloc.alloc<NodeBinExprOr>();
            if(curr_tok->type == TokenType::plus) {
                bin_expr_add->rhs = rhs.value();
                bin_expr_add->lhs = expr_lhs;
                bin_expr->var = bin_expr_add;
            }
            else if(curr_tok->type == TokenType::star) {
                bin_expr_mult->rhs = rhs.value();
                bin_expr_mult->lhs = expr_lhs;
                bin_expr->var = bin_expr_mult;
            }
            else if(curr_tok->type == TokenType::slash) {
                bin_expr_div->rhs = rhs.value();
                bin_expr_div->lhs = expr_lhs;
                bin_expr->var = bin_expr_div;
            }
            else if(curr_tok->type == TokenType::minus) {
                bin_expr_sub->rhs = rhs.value();
                bin_expr_sub->lhs = expr_lhs;
                bin_expr->var = bin_expr_sub;
            }
            else if(curr_tok->type == TokenType::percent) {
                bin_expr_rem->rhs = rhs.value();
                bin_expr_rem->lhs = expr_lhs;
                bin_expr->var = bin_expr_rem;
            }
            else if(curr_tok->type == TokenType::pow) {
                bin_expr_pow->rhs = rhs.value();
                bin_expr_pow->lhs = expr_lhs;
                bin_expr->var = bin_expr_pow;
            }
            else if(curr_tok->type == TokenType::comp) {
                bin_expr_equals->rhs = rhs.value();
                bin_expr_equals->lhs = expr_lhs;
                bin_expr->var = bin_expr_equals;
            }
            else if(curr_tok->type == TokenType::gt) {
                bin_expr_gt->rhs = rhs.value();
                bin_expr_gt->lhs = expr_lhs;
                bin_expr->var = bin_expr_gt;
            }
            else if(curr_tok->type == TokenType::gte) {
                bin_expr_gte->rhs = rhs.value();
                bin_expr_gte->lhs = expr_lhs;
                bin_expr->var = bin_expr_gte;
            }
            else if(curr_tok->type == TokenType::lt) {
                bin_expr_lt->rhs = rhs.value();
                bin_expr_lt->lhs = expr_lhs;
                bin_expr->var = bin_expr_lt;
            }
            else if(curr_tok->type == TokenType::lte) {
                bin_expr_lte->rhs = rhs.value();
                bin_expr_lte->lhs = expr_lhs;
                bin_expr->var = bin_expr_lte;
            }
            else if(curr_tok->type == TokenType::or_) {
                bin_expr_or->rhs = rhs.value();
                bin_expr_or->lhs = expr_lhs;
                bin_expr->var = bin_expr_or;
            }
            else if(curr_tok->type == TokenType::and_) {
                bin_expr_and->rhs = rhs.value();
                bin_expr_and->lhs = expr_lhs;
                bin_expr->var = bin_expr_and;
            }
            temp_expr->var = bin_expr;
            expr_lhs = temp_expr;
        }

        return expr_lhs;
    }

    optional<NodeStmtExit*> parse_exit(){
        auto node_exit = m_alloc.alloc<NodeStmtExit>();
        if(auto node_expr=parse_expr(0)) {
            auto expr = m_alloc.alloc<NodeExpr>();
            node_exit->expr=node_expr.value();
            return node_exit;
        } else {
            throw_exit_failure("Invalid expression : Exit code not found!");
        }
        return {};
    }

    optional<NodeStmtLet*> parse_let() {
        auto let = m_alloc.alloc<NodeStmtLet>();
        if(!peek().has_value() || peek().value().type != TokenType::ident
                || !peek(1).has_value() && peek(1).value().type != TokenType::eq
                || !peek(2).has_value() && peek(2).value().type != TokenType::int_lit)
        {
            throw_exit_failure("Invalid assignment to the variable!");
        }
        Token id = consume();
        consume();
        if(auto node_expr=parse_expr(0)) {
            let->expr=node_expr.value();
            let->ident=id;
            return let;
        }
        else {
            throw_exit_failure("Invalid expression : Variable identifier not found!");
        }
        return {};
    }

    optional<NodeStmtIdent*> parse_ident() {
        auto ident = m_alloc.alloc<NodeStmtIdent>();
        Token id = consume();
        ident->ident=id;

        if(peek().has_value() && peek().value().type == TokenType::plus &&
            peek(1).has_value() && peek(1).value().type == TokenType::eq)
        {
            consume();
            consume();
            if(auto node_expr=parse_expr(0)) {
                auto node_bin = m_alloc.alloc<NodeBinExpr>();
                auto node_exp2 = m_alloc.alloc<NodeExpr>();
                auto node_bin_add = m_alloc.alloc<NodeBinExprAdd>();
                auto lhs = m_alloc.alloc<NodeExpr>();
                auto term = m_alloc.alloc<NodeTerm>();
                auto term_ident = m_alloc.alloc<NodeTermIdent>();
                term_ident->ident = id;
                term->var=term_ident;
                lhs->var=term;
                node_bin_add->lhs=lhs;
                node_bin_add->rhs=node_expr.value();
                node_bin->var=node_bin_add;
                node_exp2->var=node_bin;
                ident->var=node_exp2;
                return ident;
            }
        }


        if(peek().has_value() && peek().value().type == TokenType::minus &&
            peek(1).has_value() && peek(1).value().type == TokenType::eq)
        {
            consume();
            consume();
            if(auto node_expr=parse_expr(0)) {
                auto node_bin = m_alloc.alloc<NodeBinExpr>();
                auto node_exp2 = m_alloc.alloc<NodeExpr>();
                auto node_bin_sub = m_alloc.alloc<NodeBinExprSub>();
                auto lhs = m_alloc.alloc<NodeExpr>();
                auto term = m_alloc.alloc<NodeTerm>();
                auto term_ident = m_alloc.alloc<NodeTermIdent>();
                term_ident->ident = id;
                term->var=term_ident;
                lhs->var=term;
                node_bin_sub->lhs=lhs;
                node_bin_sub->rhs=node_expr.value();
                node_bin->var=node_bin_sub;
                node_exp2->var=node_bin;
                ident->var=node_exp2;
                return ident;
            }
        }

        if(peek().has_value() && peek().value().type == TokenType::plus &&
            peek(1).has_value() && peek(1).value().type == TokenType::plus)
        {
            consume();
            consume();
            auto node_incr = m_alloc.alloc<NodeStmtIdentInc>();
            ident->var=node_incr;
            return ident;
        }

        if(peek().has_value() && peek().value().type == TokenType::minus &&
            peek(1).has_value() && peek(1).value().type == TokenType::minus)
        {
            consume();
            consume();
            auto node_decr = m_alloc.alloc<NodeStmtIdentDec>();
            ident->var=node_decr;
            return ident;
        }

        if(!peek().has_value() || peek().value().type != TokenType::eq) {
            cerr<<"Line "<< line_ct << " : " <<"Invalid assignment to the variable : " << id.value.value() <<endl;
            exit(EXIT_FAILURE);
        }
        consume();

        if(auto node_expr=parse_expr(0)) {
            ident->var=node_expr.value();
            return ident;
        }
        else {
            throw_exit_failure("Invalid expression : Variable identifier not found!");
        }
        return {};
    }

    optional<NodeScope*> parse_scope() {
        auto scope_node = m_alloc.alloc<NodeScope>();
        if(peek().has_value() && peek().value().type == TokenType::open_curly_paren) {
            line_ct++;
            consume();
            while(peek().has_value() && peek().value().type != TokenType::close_curly_paren) {
                if(auto stmt=parse_stmt()) {
                    scope_node->stmts.push_back(stmt.value());
                }
            }
            consume();
        }
        else if(auto stmt = parse_stmt()) {
            scope_node->stmts.push_back(stmt.value());
        }
        else {
            throw_exit_failure("Invalid scope statement!");
        }
        wasScope = true;
        return scope_node;
    }

    optional<NodeStmtIf*> parse_if()
    {
        auto stmt_if = m_alloc.alloc<NodeStmtIf>();
        if(auto expr = parse_expr(0)) {
            stmt_if->expr = expr.value();
        }
        else throw_exit_failure("Invalid expression for if statement!");
        if(auto if_scope = parse_scope()) {
            stmt_if->stmts = if_scope.value();
        }
        if(peek().has_value() && peek().value().type == TokenType::else_) {
            consume();
            if(auto else_scope = parse_scope()) {
                stmt_if->else_stmts = else_scope.value();
            }
        }
        return stmt_if;
    }

    optional<NodeStmtRep*> parse_rep() {
        auto node_rep = m_alloc.alloc<NodeStmtRep>();
        if(auto expr = parse_expr(0)) {
            node_rep->expr=expr.value();
        }
        if(auto scope = parse_scope()) {
            node_rep->stmts=scope.value();
        }
        return node_rep;
    }

    optional<vector<NodeExpr*>> parse_para() {
        if(peek().has_value()&&peek().value().type==TokenType::open_squ_paren) consume();
        else throw_exit_failure("Invalid function calling");
        vector<NodeExpr*> terms;
        if(auto term = parse_expr(0)) terms.push_back(term.value());
        else throw_exit_failure("Invalid function calling (weird arguments)");
        while(peek().has_value()&&peek().value().type == TokenType::comma) {
            consume();
            if(auto term = parse_expr(0)) terms.push_back(term.value());
            else throw_exit_failure("Invalid function calling (weird arguments)");
        }
        if(peek().has_value()&&peek().value().type==TokenType::close_squ_paren) consume();
        else throw_exit_failure("Invalid function calling");
        return terms;
    }

    optional<vector<Token>> parse_tokens() {
        vector<Token> terms;
        if(peek().has_value()&&peek().value().type==TokenType::open_squ_paren) consume();
        else throw_exit_failure("Invalid function declaration");
        if(peek().has_value() && peek().value().type == TokenType::ident) terms.push_back(consume());
        else throw_exit_failure("Invalid function declaration");
        while(peek().has_value()&&peek().value().type == TokenType::comma) {
            consume();
            if(peek().has_value() && peek().value().type == TokenType::ident) terms.push_back(consume());
            else throw_exit_failure("Invalid function declaration");
        }
        if(peek().has_value()&&peek().value().type==TokenType::close_squ_paren) consume();
        else throw_exit_failure("Invalid function declaration");
        return terms;
    }

    optional<NodeStmtFuncDec*> parse_func_dec() {
        auto node_func_dec = m_alloc.alloc<NodeStmtFuncDec>();

        if(peek().has_value() && peek().value().type == TokenType::ident) node_func_dec->ident = consume();
        else throw_exit_failure("Invalid function declaration");

        if(auto parameters = parse_tokens()) {
            node_func_dec->parameters = parameters.value();
        }

        if(auto scope = parse_scope()) {
            node_func_dec->stmts=scope.value();
        }
        return node_func_dec;
    }

    optional<NodeStmt*> parse_stmt()
    {
        auto stmt = m_alloc.alloc<NodeStmt>();
        bool stmt_empty = true;
        if(peek().value().type == TokenType::exit) {
            consume();
            if(auto node_exit=parse_exit()){
                stmt->var = node_exit.value();
                stmt_empty = false;
            }
        }
        else if(peek().value().type == TokenType::ret_) {
            consume();
            auto node_ret = m_alloc.alloc<NodeStmtRet>();
            if(auto node_expr=parse_expr(0)){
                node_ret->expr = node_expr.value();
                stmt->var = node_ret;
                stmt_empty = false;
            }
        }
        else if(peek().value().type == TokenType::rep) {
            consume();
            if(auto node_rep=parse_rep()){
                stmt->var = node_rep.value();
                stmt_empty = false;
            }
        }
        else if(peek().value().type == TokenType::let) {
            consume();
            if(auto node_let=parse_let()) {
                stmt->var = node_let.value();
                stmt_empty = false;
            }
        }
        else if(peek().value().type == TokenType::ident) {
            if(auto node_ident=parse_ident()) {
                stmt->var = node_ident.value();
                stmt_empty = false;
            }
        }
        else if(peek().value().type == TokenType::if_) {
            consume();
            if(auto node_if=parse_if()) {
                stmt->var = node_if.value();
                stmt_empty = false;
                // cout<<node_if.value()->stmts.size()<<endl;
            }
            else throw_exit_failure("Unable to parse if statement!");
        }
        if(peek().value().type == TokenType::open_curly_paren) {
            if(auto scope_node = parse_scope()) {
                stmt->var = scope_node.value();
                stmt_empty = false;
            }
        }

        if(bracket_Ct != 0) {
            throw_exit_failure("Invalid expression : Invalid Parenthesis");
        }
        if(peek().has_value() && peek().value().type == TokenType::semi) {
            consume();
        }
        else if(wasScope) wasScope = false;
        else {
            if(peek().has_value() && peek().value().type == TokenType::open_paren || peek().value().type == TokenType::close_paren) {
                throw_exit_failure("Invalid expression : Invalid Parenthesis");
            }
            throw_exit_failure("Invalid expression : missing ';'");
        }
        bracket_Ct = 0;
        line_ct++;
        if(stmt_empty) return {};
        else return stmt;
    }

    optional<NodeProg*> parse() {
        auto prog = m_alloc.alloc<NodeProg>();
        while(peek().has_value()) {
            if(peek().value().type == TokenType::func) {
                consume();
                if(auto node_func_dec = parse_func_dec()) {
                    prog->functions.push_back(node_func_dec.value());
                }
            }
            else if(auto stmt_node = parse_stmt()){
                prog->stmts.push_back(stmt_node.value());
            }
        }
        m_index=0;
        return prog;
    }
private:
    [[nodiscard]] optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) return {};
        return m_tokens.at(m_index + offset);
    }

    void throw_exit_failure(const string& s) const {
        cerr << "Line "<< line_ct << " : " << s << endl;
        exit(EXIT_FAILURE);
    }

    Token consume() {
        return m_tokens.at(m_index++);
    }
    const vector<Token> m_tokens;
    size_t m_index=0;   // Could've used int ind=0;
    size_t line_ct=1;
    int bracket_Ct=0;
    bool wasScope = false;
    ArenaAllocator m_alloc;
};