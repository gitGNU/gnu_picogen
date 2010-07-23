#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <boost/optional.hpp>
#include <algorithm>

using boost::optional;

template <typename T> struct remove_pointer {
        typedef T type;
};
template <typename T> struct remove_pointer<T*> {
        typedef T type;
};

class CodeIterator {
        const char *curr;
        unsigned int row_, column_;
        void inc() {
                if (*curr == '\n') {
                        ++row_;
                        column_ = 0;
                } else {
                        ++column_;
                }
                ++curr;
        }
public:
        CodeIterator()
        : curr()
        , row_(0)
        , column_(0) {}

        CodeIterator(char const * val)
        : curr(val)
        , row_(0)
        , column_(0)
        {}

        CodeIterator(CodeIterator const &rhs)
        : curr(rhs.curr)
        , row_(rhs.row_)
        , column_(rhs.column_)
        {}

        bool operator!= (CodeIterator const &rhs) {
                return row_ != rhs.row_
                    || column_ != rhs.column_
                    ;
        }
        CodeIterator operator= (CodeIterator const &rhs) {
                curr = rhs.curr;
                row_ = rhs.row_;
                column_ = rhs.column_;
                return *this;
        }
        CodeIterator& operator ++ () {
                inc();
                return *this;
        }
        CodeIterator operator ++ (int) {
                CodeIterator ret = *this;
                inc();
                return ret;
        }
        char peek (unsigned int p) {
                CodeIterator ret = *this;
                while (p!=0) {
                        ++ret;
                        --p;
                }
                return *ret;
        }
        bool can_peek (unsigned int peek) {
                CodeIterator ret = *this;
                peek--; // <-- This is so that \0 can not be peeked
                while ((peek!=0) && (*ret!='\0')) {
                        ++ret;
                        --peek;
                }
                return peek == 0;
        }
        CodeIterator next() const {
                return ++CodeIterator(*this);
        }
        char operator *() const {
                return *curr;
        }

        unsigned int row() const { return row_; }
        unsigned int column() const { return column_; }
};

bool is_num(char c) {
        switch (c) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
                return true;
        };
        return false;
}

bool is_lower_alpha (char c) {
        switch (c) {
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
                return true;
        };
        return false;
}
bool is_upper_alpha (char c) {
        switch (c) {
        case 'A': case 'B': case 'C': case 'D':
        case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
                return true;
        };
        return false;
}
bool is_alpha(char c) {
        return is_lower_alpha(c) || is_upper_alpha(c);
}
bool is_alnum(char c) {
        return is_alpha(c) || is_num(c);
}
bool is_white (char c) {
        switch (c) {
        case ' ': case '\n': case '\t': case '\r':
                return true;
        };
        return false;
}




class Token {
public:
        enum Type {
                LessThan, GreaterThan,
                TransformTo, // -->
                Colon, Semicolon, Comma,
                LeftParen, RightParen,
                Identifier, Real, Integer
        };

        Token (Type type_,
               const CodeIterator &from,
               const CodeIterator &to
        )
        : type_(type_)
        , from_(from)
        , to_(to)
        , value_("")
        {
                for (CodeIterator curr=from_; curr!=to_; ++curr) {
                        value_ += *curr;
                }
        }

        Token (Token const &rhs)
        : type_(rhs.type_)
        , from_(rhs.from_)
        , to_(rhs.to_ )
        , value_(rhs.value_)
        {}

        Token& operator= (Token const &rhs) {
                type_ = rhs.type_;
                from_ = rhs.from_;
                to_ = rhs.to_;
                value_ = rhs.value_;
                return *this;
        }

        Type type() const { return type_; }
        std::string value() const { return value_; }
        double valueAsReal () const {
                std::stringstream ss;
                ss << value_;
                double ret;
                ss >> ret;
                return ret;
        }
        int valueAsInteger () const {
                std::stringstream ss;
                ss << value_;
                int ret;
                ss >> ret;
                return ret;
        }
        CodeIterator from() const { return from_; }
        CodeIterator to() const { return to_; }
private:
        Token();
        Type type_;
        CodeIterator from_, to_;
        std::string value_;
};


typedef std::vector<Token> TokenVector;
typedef TokenVector::const_iterator TokenIterator;

TokenVector tokenize(const char *code) {
        TokenVector tokens;
        typedef CodeIterator CI;
        for (CI it=code; *it!='\0'; ++it) {
                const char c = *it;
                if (is_alpha(c)) {
                        const CI begin = it;
                        CI prev = it;
                        for (; *it!='\0' && is_alnum(*it); prev=it, ++it) {
                        }
                        tokens.push_back (Token(Token::Identifier, begin, it));
                        it = prev;
                } else if (is_num(c)) {
                        const CI begin = it;
                        CI prev = it;
                        for (; *it!='\0' && is_num(*it); prev=it, ++it) {
                        }
                        if (*it == '.') {
                                ++it;
                                if (!is_num(*it)) {
                                        std::cerr << "tokenization error in "
                                             << "line " << it.row()
                                             << ", column " << it.column()
                                             << ": '" << *it << "', "
                                             << "expected number" << std::endl;
                                        return TokenVector();
                                }
                                for (; *it!='\0' && is_num(*it); prev=it, ++it) {
                                }
                                tokens.push_back (Token(Token::Real, begin, it));
                                it = prev;
                        } else {
                                tokens.push_back (Token(Token::Integer, begin, it));
                                it = prev;
                        }
                } else if (c == '<') {
                        tokens.push_back (Token(Token::LessThan, it, it.next()));
                } else if (c == '>') {
                        tokens.push_back (Token(Token::GreaterThan, it, it.next()));
                } else if (c == '-'
                        && it.can_peek(1) && it.peek(1)=='-'
                        && it.can_peek(2) && it.peek(2)=='>'
                ) {
                        const CI begin = it;
                        ++it; ++it;
                        tokens.push_back (Token(Token::TransformTo, begin, it.next()));
                } else if (c == ':') {
                        tokens.push_back (Token(Token::Colon, it, it.next()));
                } else if (c == ';') {
                        tokens.push_back (Token(Token::Semicolon, it, it.next()));
                } else if (c == '(') {
                        tokens.push_back (Token(Token::LeftParen, it, it.next()));
                } else if (c == ')') {
                        tokens.push_back (Token(Token::RightParen, it, it.next()));
                } else if (c == ',') {
                        tokens.push_back (Token(Token::Comma, it, it.next()));
                } else if (!is_white(c)) {
                        std::cerr << "tokenization error in "
                             << "line " << it.row() << ", column " << it.column()
                             << ": '" << *it << "'" << std::endl;
                        return TokenVector();
                }
        }
        return tokens;
}


class Parameter {
public:
        enum Type {
                Identifier,
                Integer,
                Real
                //Expression
        };

        Type type() const {
                return type_;
        }
        void setType (Type type) {
                type_ = type;
        }
private:
        Type type_;
};
inline std::ostream& operator<< (std::ostream& o, Parameter const& rhs) {
        o << " : ";
        switch (rhs.type()) {
        case Parameter::Identifier: o << "id"; break;
        case Parameter::Integer:    o << "int"; break;
        case Parameter::Real: o << "real"; break;
        }
        return o;
}

class ParameterList {
public:
        unsigned int size() const {
                return parameters_.size();
        }
        void push_back(Parameter const &sym) {
                parameters_.push_back(sym);
        }
        bool empty() const {
                return size() == 0;
        }

        Parameter &operator [] (unsigned int i) {
                return parameters_[i];
        }
        const Parameter &operator [] (unsigned int i) const {
                return parameters_[i];
        }

        ParameterList() {}
        ParameterList(ParameterList const &rhs)
        : parameters_(rhs.parameters_)
        {}
        ParameterList& operator= (ParameterList const &rhs) {
                parameters_ = rhs.parameters_;
                return *this;
        }
private:
        std::vector<Parameter> parameters_;
};
inline std::ostream& operator<< (std::ostream& o, ParameterList const& rhs) {
        if (rhs.size()) {
                o << "(";
                for (unsigned int i=0; i<rhs.size(); ++i) {
                        if (i) o << ", ";
                        o << rhs[i];
                }
                o << ")";
        }
        return o;
}

class Symbol {
public:
        std::string name() const {
                return name_;
        }
        void setName(std::string const &name) {
                name_ = name;
        }

        ParameterList parameterList() const {
                return parameterList_;
        }
        void setParameterList(ParameterList const &rhs) {
                parameterList_ = rhs;
        }

        Symbol() {}
        Symbol (Symbol const &rhs)
        : name_(rhs.name_)
        , parameterList_(rhs.parameterList_)
        {}
        Symbol& operator= (Symbol const &rhs) {
                name_ = rhs.name_;
                parameterList_ = rhs.parameterList_;
                return *this;
        }
private:
        std::string name_;
        ParameterList parameterList_;
};
inline bool operator == (Symbol const &lhs, Symbol const &rhs) {
        return lhs.name() == rhs.name()
            && lhs.parameterList().size() == rhs.parameterList().size();
}
inline bool operator != (Symbol const &lhs, Symbol const &rhs) {
        return !(lhs == rhs);
}
inline std::ostream& operator<< (std::ostream& o, Symbol const& rhs) {
        o << rhs.name();
        o << rhs.parameterList();
        return o;
}


class Pattern {
public:
        Symbol const& operator[] (unsigned int i) const {
                return symbols[i];
        }
        Symbol & operator[] (unsigned int i) {
                return symbols[i];
        }
        unsigned int size() const {
                return symbols.size();
        }
        void push_back(Symbol const &sym) {
                symbols.push_back(sym);
        }
        bool empty() const {
                return size() == 0;
        }

        Pattern() {}
        Pattern(Pattern const &rhs)
        : symbols(rhs.symbols)
        {}
        Pattern& operator= (Pattern const &rhs) {
            symbols = rhs.symbols;
            return *this;
        }
private:
        std::vector<Symbol> symbols;
};
inline bool operator == (Pattern const &lhs, Pattern const &rhs) {
        if (lhs.size() != rhs.size())
                return false;
        for (unsigned int i=0; i<lhs.size(); ++i) {
                if (lhs[i] != rhs[i])
                        return false;
        }
        return true;
}
inline std::ostream& operator<< (std::ostream& o, Pattern const& rhs) {
        if (rhs.size()) {
                o << rhs[0];
        }
        for (unsigned int i=1; i<rhs.size(); ++i) {
                o << " " << rhs[i];
        }
        return o;
}


class ProductionHeader {
public:
        std::string name() const {
                return name_;
        }
        void setName(std::string const &name) {
                name_ = name;
        }

        Pattern leftContext() const {
                return leftContext_;
        }
        void setLeftContext(Pattern const & p) {
                leftContext_ = p;
        }

        Pattern rightContext() const {
                return rightContext_;
        }
        void setRightContext(Pattern const & p) {
                rightContext_ = p;
        }

        Pattern pattern() const {
                return pattern_;
        }
        void setPattern(Pattern const & p) {
                pattern_ = p;
        }

        ProductionHeader() {}

        ProductionHeader (ProductionHeader const &rhs)
        : name_(rhs.name_)
        , leftContext_(rhs.leftContext_)
        , rightContext_(rhs.rightContext_)
        , pattern_(rhs.pattern_)
        {}

        ProductionHeader& operator= (ProductionHeader const &rhs) {
                name_ = rhs.name_;
                leftContext_ = rhs.leftContext_;
                rightContext_ = rhs.rightContext_;
                pattern_ = rhs.pattern_;
                return *this;
        }

private:
        std::string name_;
        Pattern leftContext_, rightContext_;
        Pattern pattern_;
};
inline std::ostream& operator<< (std::ostream& o, ProductionHeader const& rhs) {
        o << rhs.name() << ": ";

        if (!rhs.leftContext().empty())
                o << rhs.leftContext() << " < ";

        o << rhs.pattern();

        if (!rhs.rightContext().empty())
                o << " > " << rhs.rightContext();
        return o;
}

class ProductionBody {
public:
        Pattern pattern() const {
                return pattern_;
        }
        void setPattern(Pattern const & p) {
                pattern_ = p;
        }

        ProductionBody() {}
        ProductionBody(ProductionBody const &rhs)
        : pattern_(rhs.pattern_)
        {}
        ProductionBody& operator= (ProductionBody const &rhs) {
            pattern_ = rhs.pattern_;
            return *this;
        }
private:
        Pattern pattern_;
};
inline std::ostream& operator<< (std::ostream& o, ProductionBody const& rhs) {
        o << rhs.pattern();
        return o;
}

class Production {
public:
        ProductionHeader header() const {
                return header_;
        }
        void setHeader(ProductionHeader const &h) {
                header_ = h;
        }

        ProductionBody body() const {
                return body_;
        }
        void setBody(ProductionBody const &h) {
                body_ = h;
        }

        Production() {}
        Production(Production const &rhs)
        : header_(rhs.header_)
        , body_(rhs.body_)
        {}
        Production& operator= (Production const &rhs) {
                header_ = rhs.header_;
                body_ = rhs.body_;
                return *this;
        }
private:
        ProductionHeader header_;
        ProductionBody body_;
};
inline std::ostream& operator<< (std::ostream& o, Production const& rhs) {
        o << rhs.header() << " --> " << rhs.body() << ";";
        return o;
}
inline bool hasPrecedenceOver (Production const &lhs, Production const &rhs) {
        const int numContextsLhs =
                !lhs.header().leftContext().empty()
                + !lhs.header().rightContext().empty();
        const int numContextsRhs =
                !rhs.header().leftContext().empty()
                + !rhs.header().rightContext().empty();

        // Always prefer contexts over no context, as well as
        // two contexts over one context.
        if (numContextsLhs > numContextsRhs)
                return true;
        else if (numContextsLhs < numContextsRhs)
                return false;

        // Always prefer larger main-pattern.
        const int patternSizeLhs = lhs.header().pattern().size();
        const int patternSizeRhs = rhs.header().pattern().size();
        if (patternSizeLhs > patternSizeRhs)
                return true;
        else if (patternSizeLhs < patternSizeRhs)
                return false;

        // Prefer larger context.
        const int contextSizeLhs = lhs.header().leftContext().size()
                                 + lhs.header().rightContext().size();
        const int contextSizeRhs = rhs.header().leftContext().size()
                                 + rhs.header().rightContext().size();
        if (contextSizeLhs > contextSizeRhs)
                return true;
        else if (contextSizeLhs < contextSizeRhs)
                return false;

        return false;
}
inline bool ambiguous (Production const &lhs, Production const &rhs) {
        return lhs.header().pattern() == rhs.header().pattern()
            && !hasPrecedenceOver(lhs, rhs)
            && !hasPrecedenceOver(rhs, lhs)
            ;
}


optional<Parameter> parse_parameter (
        TokenIterator it, TokenIterator end, TokenIterator &behind
) {
        if (it == end)
                return optional<Parameter>();
        if (it->type() == Token::Identifier) {
                Parameter p;
                p.setType (Parameter::Identifier);
                behind = ++it;
                return p;
        }
        if (it->type() == Token::Real) {
                Parameter p;
                p.setType (Parameter::Real);
                behind = ++it;
                return p;
        }
        if (it->type() == Token::Integer) {
                Parameter p;
                p.setType (Parameter::Integer);
                behind = ++it;
                return p;
        }
        return optional<Parameter>();
}


optional<ParameterList> parse_parameter_list (
        TokenIterator it, TokenIterator end, TokenIterator &behind
) {
        ParameterList ret;
        if (it == end || it->type() != Token::LeftParen) {
                behind = it;
                return ret;
        }
        ++it;
        if (it == end) {
                std::cerr << "error: unclosed parameter list in line "
                        << (it-1)->to().next().row() << ", column "
                        << (it-1)->to().next().column() << std::endl;
                return optional<ParameterList>();
        }

        if (it->type() == Token::RightParen) {
                ++it;
        } else while (1) {
                TokenIterator behindParam;
                optional<Parameter> p = parse_parameter(it, end, behindParam);
                if (p) {
                        ret.push_back(*p);
                        it = behindParam;
                } else {
                        std::cerr <<
                           "error: expected parameter in line "
                           << it->from().next().row() << ", column "
                           << it->from().next().column() << std::endl;
                        return optional<ParameterList>();
                }

                if (it == end) {
                        TokenIterator i = it - 1;
                        std::cerr <<
                           "error: unclosed parameter list in line "
                           << i->from().next().row() << ", column "
                           << i->from().next().column() << std::endl;
                        return optional<ParameterList>();
                } else if (it->type() == Token::RightParen) {
                        ++it;
                        break;
                } else if (it->type() == Token::Comma) {
                        ++it;
                }
        }

        behind = it;
        return ret;
}


optional<Symbol> parse_symbol (TokenIterator it, TokenIterator end, TokenIterator &behind) {
        if (it == end || it->type() != Token::Identifier)
                return optional<Symbol>();
        Symbol sym;
        sym.setName (it->value());
        ++it;

        TokenIterator behindParams;
        optional<ParameterList> params = parse_parameter_list(it, end, behindParams);
        if (!params)
                return optional<Symbol>();
        sym.setParameterList (*params);
        it = behindParams;
        behind = it;
        return sym;
}

Pattern parse_pattern (TokenIterator it, TokenIterator end, TokenIterator &behind) {
        Pattern ret;
        while (optional<Symbol> sym = parse_symbol(it, end, behind)) {
                it = behind;
                ret.push_back(*sym);
        }
        behind = it;
        return ret;
}



bool parse_match_patterns (
        TokenIterator it, TokenIterator end,
        TokenIterator &behind,
        Pattern &leftContext, Pattern &mainPattern, Pattern &rightContext
) {
        const TokenIterator beforePatterns = it;
        TokenIterator behindPattern;
        const Pattern pat = parse_pattern(it, end, behindPattern);
        TokenIterator prev = it;
        it = behindPattern;

        if (it != end && it->type() == Token::LessThan) {
                leftContext = pat;
                ++it;
                mainPattern = parse_pattern(it, end, behindPattern);
                prev = it; it = behindPattern;

                if (it != end && it->type() == Token::GreaterThan) {
                        ++it;
                        rightContext = parse_pattern(it, end, behindPattern);
                        prev = it; it = behindPattern;
                }

        } else if (it != end && it->type() == Token::GreaterThan) {
                mainPattern = pat;
                ++it;
                rightContext = parse_pattern(it, end, behindPattern);
                prev = it; it = behindPattern;
        } else if (!pat.empty()) {
                mainPattern = pat;
        }

        if (mainPattern.empty()) {
                TokenIterator i = beforePatterns - 1;
                std::cerr <<
                   "error: no pattern found in line "
                   << i->from().next().row() << ", column "
                   << i->from().next().column() << std::endl;
                return optional<Production>();
        }
        behind = it;

        return true;
}

optional<ProductionHeader> parse_production_header (
        TokenIterator it, TokenIterator end, TokenIterator &behind
) {
        TokenIterator prev = it;

        if (Token::Identifier != it->type()) {
                std::cerr << "error: expected identifier at line "
                        << prev->from().row() << ", column "
                        << prev->from().column() << std::endl;
                return optional<ProductionHeader>();
        }
        const std::string name = it->value();
        prev = it;
        ++it;
        // Checkpoint: We have an identifier.

        if (it==end || it->type() != Token::Colon) {
                std::cerr << "error: expected ':' at line "
                   << prev->to().next().row() << ", column "
                   << prev->to().next().column() << std::endl;
                return optional<ProductionHeader>();
        }
        prev = it;
        ++it;
        // Checkpoint: The identifier is followed by a colon.

        TokenIterator behindMatchPatterns;
        Pattern leftContext, mainPattern, rightContext;

        if (!parse_match_patterns(it, end, behindMatchPatterns,
                                  leftContext, mainPattern, rightContext)
        ) {
                return optional<ProductionHeader>();
        }
        it = behindMatchPatterns;

        if (it == end || it->type() != Token::TransformTo) {
                TokenIterator i = it - 1;
                std::cerr <<
                   "error: expected body (starting with '-->') for "
                   " production in line "
                   << i->from().next().row() << ", column "
                   << i->from().next().column() << ", but found "
                   << "'" << i->value() << "'" << std::endl;
                return optional<ProductionHeader>();
        }
        // Checkpoint: We just passed "-->"

        ++it;
        behind = it;

        ProductionHeader ret;
        ret.setName (name);
        ret.setLeftContext(leftContext);
        ret.setRightContext(rightContext);
        ret.setPattern(mainPattern);
        return ret;
}

optional<ProductionBody> parse_production_body (
        // foo --> bar  x:
        TokenIterator it, TokenIterator end, TokenIterator &behind
) {
        TokenIterator behindPattern;
        const Pattern pat = parse_pattern(it, end, behindPattern);
        it = behindPattern;
        if (it == end || it->type() != Token::Semicolon) {
                TokenIterator prev = it - 1;
                std::cerr << "error: expected ';' after production in line "
                        << prev->to().row() << ", column "
                        << prev->to().column() << std::endl;
                return optional<ProductionBody>();
        }
        ++it;
        behind = it;

        ProductionBody ret;
        ret.setPattern (pat);
        return ret;
}

optional<Production> parse_production (TokenIterator it, TokenIterator end, TokenIterator &behind) {
        if (it==end) {
                std::cerr << "found nothing" << std::endl;
                return optional<Production>();
        }


        optional<ProductionHeader> header;
        optional<ProductionBody> body;

        // Header
        {
                TokenIterator behindHeader;
                header = parse_production_header(it, end, behindHeader);
                if (!header) {
                        return optional<Production>();
                }
                it = behindHeader;

                if (it == end) {
                        TokenIterator i = it - 1;
                        std::cerr <<
                           "error: expected body behind '-->' in line "
                           << i->to().next().row() << ", column "
                           << i->to().next().column() << std::endl;
                        return optional<Production>();
                }
        }
        // Body
        {
                TokenIterator behindBody;
                body = parse_production_body(it, end, behindBody);
                if (!body) {
                        return optional<Production>();
                }
                it = behindBody;
        }


        behind = it;

        Production ret;
        ret.setHeader (*header);
        ret.setBody (*body);
        return ret;
}

void compile (const char *code) {
        const TokenVector tokens = tokenize (code);

        std::vector<Production> prods;

        std::set<std::string> names;
        TokenIterator it = tokens.begin();
        const TokenIterator &end = tokens.end();
        //char c = 'a';
        while (it != end) {
                TokenIterator behind;
                if (optional<Production> op = parse_production (it, end, behind)) {
                        if (names.count(op->header().name())) {
                                std::cerr << "error: multiple productions are "
                                 << "named '" << op->header().name() << "'.\n";
                                return;
                        }
                        names.insert(op->header().name());
                        prods.push_back(*op);
                } else {
                        return;
                }
                it = behind;
        }

        std::stable_sort (prods.begin(), prods.end(), hasPrecedenceOver);
        for (unsigned int i=0; i<prods.size(); ++i) {
                std::cout << i << " -- " << prods[i] << '\n';
        }

        // ABoP seems to allow for overloading, i.e. "b" != "b(x)".
        // I guess that can cause confusion, thus we'll try to warn
        // about overloads.
        std::map<std::string, Symbol> first_appearance;
        for (unsigned int i=0; i<prods.size(); ++i) {

                // Check if there are ambiguous rules which could lead
                // to a situation like:
                //  axiom: a   b   a
                //      A: a < b
                //      B:     b < a
                if (i<prods.size()-1
                   && ambiguous (prods[i], prods[i+1])
                ) {
                        std::cerr << "warning: productions '"
                          << prods[i].header().name()
                          << "' and '"
                          << prods[i+1].header().name()
                          << "' might be ambiguous (neither is more "
                          << "specialized); in such case, '"
                          << prods[i].header().name()
                          << "' will be preferred (because it was "
                          << "declared first).\n";
                        ;
                }

                const Pattern pats[4] = {
                        prods[i].header().leftContext(),
                        prods[i].header().pattern(),
                        prods[i].header().rightContext(),
                        prods[i].body().pattern()
                };
                for (unsigned int p=0; p<4; ++p) {
                        const Pattern &pat = pats[p];
                        for (unsigned int i=0; i<pat.size(); ++i) {
                                if (!first_appearance.count(pat[i].name())) {
                                        first_appearance[pat[i].name()] = pat[i];
                                } else if (pat[i] != first_appearance[pat[i].name()]) {
                                        Symbol const & sym =
                                               first_appearance[pat[i].name()];
                                        std::cerr
                                          << "warning: letter '"
                                          << pat[i].name() << "' used with"
                                          << " differing parameter-counts, "
                                          << "'" << pat[i] << "' won't match "
                                          << "'" << sym << "'" << std::endl;
                                }
                        }
                }
        }


        const TokenVector axiom = tokenize("b(x,z) a a a a");
        std::cout << "--------------\n";
        TokenIterator behind;
        Pattern pat = parse_pattern(axiom.begin(), axiom.end(), behind);
        std::cout << "axiom: " << pat << '\n';

        for (int step=0; step<6; ++step) {
                optional<Pattern> apply(std::vector<Production> const &, Pattern const &);

                optional<Pattern> next = apply (prods, pat);
                if (next) {
                        pat = *next;
                        std::cout << "step " << step << ": " << pat << '\n';
                } else {
                        std::cout << "no match in step " << step << '\n';
                        break;
                }
        }
}


unsigned int matchLength (Pattern const &pattern,
                 Pattern const &axiom,
                 int axiomIndex
) {
        if (axiomIndex<0)
                return 0;
        for (unsigned int i=0; i<pattern.size(); ++i) {
                if (i+axiomIndex >= axiom.size()) return 0;
                if (pattern[i] != axiom[i+axiomIndex])
                        return 0;
        }
        return pattern.size();
}

unsigned int matchLength (Production const &production,
                 Pattern const &axiom,
                 int axiomIndex
) {
        const ProductionHeader &header = production.header();
        const int mainLen = matchLength (header.pattern(), axiom, axiomIndex);
        if (0 == mainLen)
                return 0;

        if (!header.leftContext().empty()) {
                Pattern const & ct = header.leftContext();
                if (!matchLength (ct, axiom, axiomIndex-ct.size()))
                        return 0;
        }
        if (!header.rightContext().empty()) {
                Pattern const & ct = header.rightContext();
                if (!matchLength (ct, axiom, axiomIndex+mainLen))
                        return 0;
        }
        return mainLen;
}


optional<Pattern> apply(std::vector<Production> const &prods, Pattern const &axiom) {
        bool axiomWasTweaked = false;
        Pattern ret;
        for (unsigned int A=0; A<axiom.size(); ) {
                bool any = false;
                for (unsigned int P=0; P<prods.size(); ++P) {
                        const int len = matchLength(prods[P], axiom, A);
                        const bool doesMatch = len > 0;
                        if (doesMatch) {
                                A += len;
                                any = true;
                                const Pattern &body = prods[P].body().pattern();
                                for (unsigned int i=0; i<body.size(); ++i) {
                                        ret.push_back(body[i]);
                                }
                                break;
                        }
                }
                if (any) {
                        axiomWasTweaked = true;
                } else {
                        ret.push_back (axiom[A]);
                        ++A;
                }
        }

        if (axiomWasTweaked)
                return ret;
        return optional<Pattern>();
}


int main()
{
        // f(x) < y(x)   should yield an error "parameter names may only appear once"
        const char * code =
                "a0: b(x,x) < a --> b(5,5);\n"
                "a1:     b(x,x) --> a;\n"
                //"m: a --> a foo;"
        ;
        compile(code);

        return 0;
}