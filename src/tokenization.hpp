#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum class TokenType {exit,int_lit,semi,open_paren,close_paren,ident,let,eq,
    plus,minus,star,pow,slash,percent,gt,lt,gte,lte,comp,if_,open_curly_paren,
    close_curly_paren,else_,and_,or_,rep,comma,ret_,func,open_squ_paren,close_squ_paren};

struct Token {
    TokenType type;
    optional<string> value{};
};

optional<int> bin_prec(TokenType type) {
    switch (type) {
    case TokenType::plus: return 0;
    case TokenType::minus: return 0;
    case TokenType::star: return 1;
    case TokenType::slash: return 1;
    case TokenType::pow: return 2;
    case TokenType::gt: return  4;
    case TokenType::gte: return  4;
    case TokenType::lt: return 4;
    case TokenType::lte: return 4;
    case TokenType::comp: return 4;
    case TokenType::and_: return 5;
    case TokenType::or_: return 5;
    case TokenType::percent: return 3;

    default: return {};
    }
}

class Tokenizer {
public:

    explicit Tokenizer(string src)
        : m_src(move(src)){}


    inline vector<Token> tokenize(){
        vector<Token> tokens;
        string buff;
        while(ind<m_src.size()){
            if(isspace(m_src.at(ind))){
                ind++;
                continue;
            }
            if(isalpha(m_src.at(ind))){
                buff.push_back(m_src.at(ind++));
                while(isalnum(m_src.at(ind))) buff.push_back(m_src.at(ind++));
                if(buff=="exit"){
                    tokens.push_back({.type = TokenType::exit});
                    buff.clear();
                    continue;
                }
                else if(buff=="let"){
                    tokens.push_back({.type = TokenType::let});
                    buff.clear();
                    continue;
                }
                else if(buff=="if"){
                    tokens.push_back({.type = TokenType::if_});
                    buff.clear();
                    continue;
                }
                else if(buff=="else"){
                    tokens.push_back({.type = TokenType::else_});
                    buff.clear();
                    continue;
                }
                else if(buff=="rep"){
                    tokens.push_back({.type = TokenType::rep});
                    buff.clear();
                    continue;
                }
                else if(buff=="return"){
                    tokens.push_back({.type = TokenType::ret_});
                    buff.clear();
                    continue;
                }
                else if(buff=="function"){
                    tokens.push_back({.type = TokenType::func});
                    buff.clear();
                    continue;
                }
                else{
                    tokens.push_back({.type = TokenType::ident, .value = buff});
                    buff.clear();
                    continue;
                }
            }
            if(isdigit(m_src.at(ind)))
            {
                while(isdigit(m_src.at(ind))) buff.push_back(m_src.at(ind++));
                tokens.push_back({.type =TokenType::int_lit, .value = buff});
                buff.clear();
                continue;
            }
            if(m_src.at(ind)=='&') {
                if(m_src.at(ind+1)=='&') {
                    tokens.push_back({.type = TokenType::and_});
                    ind+=2;
                }
                else {
                    cerr << "Invalid operand : &" << endl;
                    exit(EXIT_FAILURE);
                }
            }
            if(m_src.at(ind)=='|') {
                if(m_src.at(ind+1)=='|') {
                    tokens.push_back({.type = TokenType::or_});
                    ind+=2;
                }
                else {
                    cerr << "Invalid operand : |" << endl;
                    exit(EXIT_FAILURE);
                }
            }
            if(m_src.at(ind)=='=')
            {
                if(m_src.at(ind+1)=='=') {
                    tokens.push_back({.type = TokenType::comp});
                    ind++;
                }
                else tokens.push_back({.type = TokenType::eq});
                ind++;
            }
            if(m_src.at(ind)==';')
            {
                tokens.push_back({.type = TokenType::semi});
                ind++;
            }
            else if(m_src.at(ind)=='[')
            {
                tokens.push_back({.type = TokenType::open_squ_paren});
                ind++;
            }
            else if(m_src.at(ind)==']')
            {
                tokens.push_back({.type = TokenType::close_squ_paren});
                ind++;
            }
            else if(m_src.at(ind)=='{')
            {
                tokens.push_back({.type = TokenType::open_curly_paren});
                ind++;
            }
            else if(m_src.at(ind)=='}')
            {
                tokens.push_back({.type = TokenType::close_curly_paren});
                // tokens.push_back({.type = TokenType::semi});
                ind++;
            }
            else if(m_src.at(ind)=='(')
            {
                tokens.push_back({.type = TokenType::open_paren});
                ind++;
            }
            else if(m_src.at(ind)==')')
            {
                tokens.push_back({.type = TokenType::close_paren});
                ind++;
            }
            else if(m_src.at(ind)=='+')
            {
                tokens.push_back({.type = TokenType::plus});
                ind++;
            }
            else if(m_src.at(ind)=='-')
            {
                tokens.push_back({.type = TokenType::minus});
                ind++;
            }
            else if(m_src.at(ind)=='*')
            {
                tokens.push_back({.type = TokenType::star});
                ind++;
            }
            else if(m_src.at(ind)=='/')
            {
                tokens.push_back({.type = TokenType::slash});
                ind++;
            }
            else if(m_src.at(ind)=='%')
            {
                tokens.push_back({.type = TokenType::percent});
                ind++;
            }
            else if(m_src.at(ind)=='^')
            {
                tokens.push_back({.type = TokenType::pow});
                ind++;
            }
            else if(m_src.at(ind)=='>')
            {
                if(m_src.at(ind+1)=='=') {
                    tokens.push_back({.type = TokenType::gte});
                    ind++;
                }
                else tokens.push_back({.type = TokenType::gt});
                ind++;
            }
            else if(m_src.at(ind)=='<')
            {
                if(m_src.at(ind+1)=='=') {
                    tokens.push_back({.type = TokenType::lte});
                    ind++;
                }
                else tokens.push_back({.type = TokenType::lt});
                ind++;
            }
            else if(m_src.at(ind)==',')
            {
                tokens.push_back({.type = TokenType::comma});
                ind++;
            }
        }
        buff.clear();
        ind=0;
        return tokens;
    }

private:
    [[nodiscard]] optional<char> peak(const int ahead=0) const{
        if(ind+ahead>=m_src.size()) return {};
        else return m_src.at(ind+ahead);
    }

    char consume(){
        return m_src.at(ind++);
    }

    const string m_src;
    size_t ind=0;
};