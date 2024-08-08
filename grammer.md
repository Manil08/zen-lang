$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^* \\
    [\text{Stmt}] &\to
    \begin{cases}
        \text{exit}([\text{Expr}]); \\
        \text{let}\space\text{ident} = [\text{Expr}]; \\
        \text{[ident]}\\
        \text{if} ([\text{Expr}])[\text{Scope}]\\
        [\text{Scope}]\\
        \text{rep} ([\text{Expr}])[\text{Scope}]\\
    \end{cases} \\
    [\text{ident}] &\to
    \begin{cases}
        [\text{Expr}] \\
        [\text{++}] \\
        [\text{ - - }] \\
    \end{cases} \\
    \text{[Scope]} &\to \{[\text{Stmt}]^*\} \\
    [\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end{cases} \\
    [\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] || [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] \&\& [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] == [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] <= [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] < [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] >= [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] > [\text{Expr}] & \text{prec} = 4 \\
        [\text{Expr}] \% [\text{Expr}] & \text{prec} = 3 \\
        [\text{Expr}] \^\ [\text{Expr}] & \text{prec} = 2 \\
        [\text{Expr}] * [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] / [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] + [\text{Expr}] & \text{prec} = 0 \\
        [\text{Expr}] - [\text{Expr}] & \text{prec} = 0 \\
    \end{cases} \\ 
    [\text{Term}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident} \\
        ([\text{Expr}])
    \end{cases}
\end{align}
$$
