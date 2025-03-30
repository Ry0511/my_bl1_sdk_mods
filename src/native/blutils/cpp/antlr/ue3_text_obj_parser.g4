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

name_literal    : SQUOTE ( qualified_identifier child_identifier? ) SQUOTE;
child_identifier: COLON identifier;

program      : begin_object* | EOF;

// Begin Object ... End Object
begin_object : KW_BEGIN KW_OBJECT
               class_identifier
               name_identifier
               ( begin_object | prop_assign | at_call )*
               KW_END KW_OBJECT
             ;

// foo.baz.bar
qualified_identifier : identifier ( DOT identifier )*;

// Class=SomeClass
class_identifier : KW_CLASS EQUAL identifier;

// Name=SomeName_12
name_identifier  : KW_NAME EQUAL qualified_identifier;

// A=B A(1)=B
prop_assign       : prop_identifier EQUAL obj_value?;
prop_identifier   : identifier prop_array_access?;
prop_array_access : LPAREN INTEGER_LITERAL RPAREN;

// @my_func(param1, param2, param3, param4)
at_call    : SYM_AT qualified_identifier LPAREN at_val_seq? RPAREN;

at_val_seq : at_value ( COMMA at_value )*;

at_value   : STRING_LITERAL
           | name_literal
           | BOOL_LITERAL
           | INTEGER_LITERAL
           | DECIMAL_LITERAL
           ;

// Right hand side of the A=obj_value
obj_value : obj_ref
          | name_tuple
          | kv_tuple
          | STRING_LITERAL
          | BOOL_LITERAL
          | INTEGER_LITERAL
          | DECIMAL_LITERAL
          | identifier
          ;

// Reference to an object either external or as a child to another object
obj_ref   : identifier LPAREN? name_literal RPAREN?;

// Tuple of names OnCreate=(Foo'Baz', Bar'Foo')
name_tuple : LPAREN ( name_val ( COMMA name_val )* ) RPAREN;
name_val   : obj_ref | KW_NONE;

// Tuple like sequence of values (assumes that A=() is invalid which I am unsure about)
kv_tuple  : LPAREN ( prop_assign (COMMA prop_assign)* )? RPAREN
          ;
