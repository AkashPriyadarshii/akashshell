pub mod lexer;
pub mod ast;
pub mod parser;

#[cfg(test)]
mod tests {
    use super::*;
    use lexer::{lex, Token};
    use ast::Expr;
    use parser::parse;

    #[test]
    fn test_lexing_and_parsing() {
        let input = "ls -la | grep \"rust\"";
        let tokens = lex(input);
        
        assert_eq!(tokens.len(), 5);
        assert_eq!(tokens[2], Token::Pipe);
        
        let ast = parse(tokens).unwrap();
        match ast {
            Expr::Pipeline(cmds) => {
                assert_eq!(cmds.len(), 2);
            }
            _ => panic!("Expected pipeline"),
        }
    }
}
