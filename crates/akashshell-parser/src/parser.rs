use crate::lexer::Token;
use crate::ast::Expr;

pub fn parse(tokens: Vec<Token>) -> Result<Expr, String> {
    let mut pipeline = Vec::new();
    let mut current_command = None;
    let mut current_args = Vec::new();

    for token in tokens {
        match token {
            Token::Word(w) | Token::StringLiteral(w) => {
                if current_command.is_none() {
                    current_command = Some(w);
                } else {
                    current_args.push(w);
                }
            }
            Token::Pipe => {
                if let Some(cmd) = current_command.take() {
                    pipeline.push(Expr::Command {
                        name: cmd,
                        args: current_args.clone(),
                    });
                    current_args.clear();
                } else {
                    return Err("Syntax Error: Empty pipeline segment".into());
                }
            }
        }
    }
    
    if let Some(cmd) = current_command.take() {
        pipeline.push(Expr::Command {
            name: cmd,
            args: current_args,
        });
    }

    if pipeline.is_empty() {
        return Err("No command provided".into());
    }

    if pipeline.len() == 1 {
        Ok(pipeline.remove(0))
    } else {
        Ok(Expr::Pipeline(pipeline))
    }
}
