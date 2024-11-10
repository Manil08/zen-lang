#include <iostream>
#include <sstream>
#include <fstream>

#include "generation.hpp"
#include "parser.hpp"
#include "tokenization.hpp"

using namespace std;

int main(int argc, char* argv[])
{

    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "zen <input.zen>" << std::endl;
        return EXIT_FAILURE;
    }

    string contents;
    {
        stringstream contents_stream;
        fstream input(argv[1],ios::in);
        contents_stream<< input.rdbuf();
        contents=contents_stream.str();
    }

    Tokenizer tokenizer(move(contents));
    vector<Token> tokens=tokenizer.tokenize();

    Parser parser(move(tokens));
    auto tree = parser.parse();

    if(!tree.has_value())
    {
        cerr<<"No parsing found!"<<endl;
        exit(EXIT_FAILURE);
    }

    {
        Generator generator(tree.value());
        fstream file("out.asm",ios::out);
        string lmao=generator.gen_prog();
        file<<lmao;
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

}