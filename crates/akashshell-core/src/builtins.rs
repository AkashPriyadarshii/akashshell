use crate::Value;

/// Built-in `cd` command
pub fn execute_cd(args: &[String]) -> Result<Value, String> {
    let path = if args.is_empty() {
        "~".to_string() 
    } else {
        args[0].clone()
    };
    
    // In actual implementation, we will use std::env::set_current_dir
    Ok(Value::String(format!("Changed directory to {}", path)))
}

/// Built-in `echo` command
pub fn execute_echo(args: &[String]) -> Result<Value, String> {
    Ok(Value::String(args.join(" ")))
}

/// Built-in `ls` command (mocking for Phase 2)
pub fn execute_ls(_args: &[String]) -> Result<Value, String> {
    Ok(Value::String("Directory listing placeholder".to_string()))
}
