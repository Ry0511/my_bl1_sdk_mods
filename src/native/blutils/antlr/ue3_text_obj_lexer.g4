lexer grammar ue3_text_obj_lexer;

fragment SIGN     : [+-];
fragment DIGIT    : [0-9];
fragment HEX_DIGIT: [0-9a-fA-F];

fragment ESC_SEQ : '\\' .
                 ;

fragment FULL_LINE : ~[\r\n]*;

INTEGER_LITERAL : SIGN? DIGIT+ [xX] HEX_DIGIT+
                | DIGIT+
                ;

LINE_COMMENT    : '//' ~[\r\n]*
                -> channel(HIDDEN);

// Keep it simple here lol
DECIMAL_LITERAL : SIGN? DIGIT+ '.' DIGIT+;

// Not sure if this is absolutely correct
STRING_LITERAL: '"' (~["\\] | ESC_SEQ)* '"';

LPAREN    : '(';
RPAREN    : ')';

WS        : [ \t]+  -> channel(HIDDEN);

KW_BEGIN : 'Begin';
KW_OBJECT: 'Object';
KW_CLASS : 'Class';
KW_NAME  : 'Name';
KW_END   : 'End';

TRUE  : 'True';
FALSE : 'False';

EQUAL : '=';
MINUS : '-';
PLUS  : '+';
STAR  : '*';
SQUOTE: '\'';
DQUOTE: '"';
COMMA : ',';
DOT   : '.';
COLON : ':';

IDENTIFIER:	[a-zA-Z_][a-zA-Z0-9_]*;

CATCH_ALL : .  -> channel(HIDDEN);