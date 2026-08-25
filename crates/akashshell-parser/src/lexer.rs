#[derive(Debug, PartialEq, Clone)]
pub enum Token {
    Word(String),
    Pipe,
    StringLiteral(String),
}

pub fn lex(input: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut current_word = String::new();
    let mut chars = input.chars().peekable();

    while let Some(c) = chars.next() {
        match c {
            '|' => {
                if !current_word.is_empty() {
                    tokens.push(Token::Word(current_word.clone()));
                    current_word.clear();
                }
                tokens.push(Token::Pipe);
            }
            ' ' | '\t' => {
                if !current_word.is_empty() {
                    tokens.push(Token::Word(current_word.clone()));
                    current_word.clear();
                }
            }
            '"' => {
                let mut string_lit = String::new();
                while let Some(&next_c) = chars.peek() {
                    if next_c == '"' {
                        chars.next();
                        break;
                    }
                    string_lit.push(chars.next().unwrap());
                }
                tokens.push(Token::StringLiteral(string_lit));
            }
            _ => {
                current_word.push(c);
            }
        }
    }
    if !current_word.is_empty() {
        tokens.push(Token::Word(current_word));
    }
    tokens
}
