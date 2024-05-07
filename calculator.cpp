#include <iostream>
#include <stdexcept>
#include <map>
#include <cctype>
#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Constants for token kinds
const char number = '8'; // kind for numbers
const char quit = 'q';   // kind for quit
const char print = ';';  // kind for print
const char name = 'a';   // kind for names (variables and constants)

class token {
public:
    char kind_;         // what kind of token
    double value_;      // for numbers: a value
    std::string name_;  // for variables

    // Constructors
    token() : kind_('\0'), value_(0) {}

    token(char ch) : kind_(ch), value_(0) {}

    token(double val) : kind_(number), value_(val) {}

    token(char ch, std::string name) : kind_(ch), value_(0), name_(name) {}

    char kind() const { return kind_; }

    double value() const { return value_; }

    std::string name() const { return name_; }
};

class token_stream {
    bool full;      // is there a token in the buffer?
    token buffer;   // here is where we keep a Token put back using

public:
    token_stream() : full(false), buffer() {}  // Use default constructor

    token get();            // get a token

    void putback(token t);  // put a token back into the token_stream

    void ignore(char c);    // discard tokens up to and including a c
};

token_stream ts;

// constructor: make a token_stream, the buffer starts empty
void token_stream::putback(token t) {
    if (full) throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get() {     // read a token from the token_stream
    // check if we already have a Token ready
    if (full) {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;
    switch (ch) {
        case print:
        case quit:
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
            return token(ch);
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            std::cin.putback(ch);
            double val;
            std::cin >> val;
            return token(val);
        }
        default:
            if (isalpha(ch)) {
                std::string s;
                s += ch;
                while (std::cin.get(ch) && (isalnum(ch) || ch == '_')) s += ch;
                std::cin.putback(ch);
                return token(name, s);
            }
            throw std::runtime_error("Bad token");
    }
}

// discard tokens up to and including a c
void token_stream::ignore(char c) {
    if (full && buffer.kind() == c) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (std::cin >> ch)
        if (ch == c) return;
}

std::map<std::string, double> variables = {
        {"pi", M_PI},
        {"e",  M_E}
};

// declaration so that primary() can call expression()
double expression();

// Number or ‘(‘ Expression ‘)’
double primary() {
    token t = ts.get();
    switch (t.kind()) {
        case '(': {
            double d = expression();
            t = ts.get();
            if (t.kind() != ')') throw std::runtime_error("')' expected");
            return d;
        }
        case number:
            return t.value();
        case name: {
            token t2 = ts.get();
            if (t2.kind() == '=') {
                double d = expression();
                variables[t.name()] = d;
                return d;
            }
            ts.putback(t2);
            return variables[t.name()];
        }
        case '-':
            return -primary();
        case '+':
            return primary();
        default:
            throw std::runtime_error("Primary expected");
    }
}

// exactly like expression(), but for * and /
double term() {
    double left = primary();
    while (true) {
        token t = ts.get();
        switch (t.kind()) {
            case '*':
                left *= primary();
                break;
            case '/': {
                double d = primary();
                if (d == 0) throw std::runtime_error("Divide by zero");
                left /= d;
                break;
            }
            case '%':
            {
                double d = primary();
                if (d == 0) throw std::runtime_error("Divide by zero");
                if (static_cast<int>(left) != left || static_cast<int>(d) != d)
                    throw std::runtime_error("Modulus requires integer operands");
                left = static_cast<int>(left) % static_cast<int>(d);
                break;
            }
            default:
                ts.putback(t);
                return left;
        }
    }
}

// read and evaluate: 1   1+2.5   1+2+3.14  etc.
// 	 return the sum (or difference)
double expression() {
    double left = term();
    while (true) {
        token t = ts.get();
        switch (t.kind()) {
            case '+':
                left += term();
                break;
            case '-':
                left -= term();
                break;
            default:
                ts.putback(t);
                return left;
        }
    }
}

void clean_up_mess() {
    ts.ignore(print);
}

const std::string prompt = "> ";
const std::string result = "= ";

void calculate() {
    while (true) {
        try {
            std::cout << prompt;
            token t = ts.get();

            while (t.kind() == print) t = ts.get();

            if (t.kind() == quit) return;

            ts.putback(t);
            std::cout << result << expression() << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            clean_up_mess();
        }
    }
}

int main() {
    try {
        calculate();
        return 0;
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unhandled exception" << std::endl;
        return 2;
    }
}
