parser grammar ue3_text_obj_parser;

options {
    tokenVocab=ue3_text_obj_lexer;
}

identifier : IDENTIFIER
           | ( KW_BEGIN
             | KW_OBJECT
             | KW_CLASS
             | KW_NAME
             | KW_END
             );

program      : begin_object* | EOF;

// Begin Object ... End Object
begin_object : KW_BEGIN KW_OBJECT
               class_identifier
               name_identifier
               (begin_object | prop_assign)*
               KW_END KW_OBJECT
             ;

// Class=SomeClass
class_identifier : KW_CLASS EQUAL identifier;

// Name=SomeName_12
name_identifier  : KW_NAME EQUAL identifier;

// A=B A(1)=B
prop_assign       : prop_identifier EQUAL obj_value?;
prop_identifier   : identifier prop_array_access?;
prop_array_access : LPAREN INTEGER_LITERAL RPAREN;

// Right hand side of the A=obj_value
obj_value: obj_ref
         | kv_tuple
         | INTEGER_LITERAL
         | STRING_LITERAL
         | DECIMAL_LITERAL
         | BOOL_LITERAL
         | identifier
         ;

// Reference to an object either external or as a child to another object
obj_ref       : LPAREN? identifier SQUOTE ( root_obj_ref child_obj_ref? ) SQUOTE RPAREN?;
root_obj_ref  : identifier (DOT identifier)*;
child_obj_ref : COLON identifier;

// Tuple like sequence of values (assumes that A=() is invalid which I am unsure about)
kv_tuple: LPAREN ( prop_assign (COMMA prop_assign)* )? RPAREN
        ;
