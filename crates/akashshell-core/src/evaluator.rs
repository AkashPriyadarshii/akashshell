use crate::Value;
use crate::builtins::{execute_cd, execute_echo, execute_ls};
use akashshell_parser::ast::Expr;

/// Evaluates an AST expression, optionally taking input from a previous pipeline stage.
pub fn evaluate(expr: &Expr, input: Option<Value>) -> Result<Value, String> {
    match expr {
        Expr::Command { name, args } => {
            // For Phase 3, we only execute internal built-ins.
            // Phase 4 will introduce external EXE execution.
            match name.as_str() {
                "cd" => execute_cd(args),
                "echo" => {
                    // Example of pipeline object passing: if echo receives input, it appends it.
                    let mut final_args = args.clone();
                    if let Some(val) = &input {
                        final_args.push(val.as_string());
                    }
                    execute_echo(&final_args)
                }
                "ls" => execute_ls(args),
                _ => {
                    use akashshell_pty::conpty::ConPty;
                    let pty = ConPty::new(name, args);
                    match pty.spawn_and_capture() {
                        Ok(output) => Ok(Value::String(output.trim().to_string())),
                        Err(e) => Err(format!("Command not found or execution failed: {}", e)),
                    }
                }
            }
        }
        Expr::Pipeline(commands) => {
            let mut current_input = input;
            
            for cmd in commands {
                let result = evaluate(cmd, current_input)?;
                current_input = Some(result);
            }
            
            current_input.ok_or_else(|| "Pipeline resulted in no output".to_string())
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use akashshell_parser::lexer::lex;
    use akashshell_parser::parser::parse;

    #[test]
    fn test_pipeline_evaluation() {
        // "echo hello | echo world"
        // The first echo outputs "hello". 
        // The second echo receives "hello" as input and outputs "world hello".
        let tokens = lex("echo hello | echo world");
        let ast = parse(tokens).unwrap();
        
        let result = evaluate(&ast, None).unwrap();
        assert_eq!(result.as_string(), "world hello");
    }

    #[test]
    fn test_command_not_found() {
        let tokens = lex("fakecmd args");
        let ast = parse(tokens).unwrap();
        
        let result = evaluate(&ast, None);
        assert!(result.is_err());
        assert!(result.unwrap_err().starts_with("Command not found or execution failed"));
    }
}
