/**
 * Authors: Rene Kok & Aram Mutlu
 * Pseudo code for a top-down recursive-descent parser 
 * from the start-separated, predictive grammar of Assignment 3.3.
 */
Start() {
    return Expr4() && (nextToken () == eof);
}

Expr4() {
    return Expr3() && Expr4P();
}

Expr4P() {
    switch (token = nextToken()) {
        case Addition: return Expr4() && Expr3();
        default: unget(token);
                 return true;
    }
}

Expr3() {
    return Expr2() && Expr3P();
}

Expr3P() {
    switch (token = nextToken()) {
        case UnaryMinus: return Expr3();
        default: unget(token);
                 eturn true;
    }
}

Expr2() {
    return Expr1() && Expr2P();
}

Expr2P() {
    switch (token = nextToken()) {
        case PrefixIncrement: return Expr2();
        default: unget(token);
                 return true;
    }
}

Expr1() {
    return (nextToken() == Id) && Expr1P();
}

Expr1P() {
    switch (token = nextToken()) {
        case (: return Expr4();
        default: unget(token);
                 return true;
    }
}

Id() {
    return nextToken () == Id;
}