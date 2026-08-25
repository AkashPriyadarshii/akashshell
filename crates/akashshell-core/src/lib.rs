pub mod builtins;
pub mod evaluator;
use std::collections::HashMap;

/// The core data structure for the akashshell pipeline.
/// Instead of passing raw strings between commands, we pass strongly typed `Value`s.
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    /// A 64-bit integer
    Int(i64),
    /// A floating-point number
    Float(f64),
    /// A UTF-8 string
    String(String),
    /// A boolean value
    Bool(bool),
    /// A dictionary/object with string keys and Value values
    Record(HashMap<String, Value>),
    /// A list of Values
    List(Vec<Value>),
    /// Represents no value or null
    Nothing,
}

impl Value {
    /// Helper to represent the Value as a string for basic rendering
    pub fn as_string(&self) -> String {
        match self {
            Value::Int(i) => i.to_string(),
            Value::Float(f) => f.to_string(),
            Value::String(s) => s.clone(),
            Value::Bool(b) => b.to_string(),
            Value::Record(r) => format!("{:?}", r), // Placeholder for proper table rendering
            Value::List(l) => format!("{:?}", l),   // Placeholder for proper list rendering
            Value::Nothing => String::new(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_value_creation() {
        let val = Value::Int(42);
        assert_eq!(val.as_string(), "42");
        
        let string_val = Value::String("hello".to_string());
        assert_eq!(string_val.as_string(), "hello");
    }
}
