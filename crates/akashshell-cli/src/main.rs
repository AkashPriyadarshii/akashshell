use std::io::{self, Write};
use std::env;
use akashshell_parser::lexer::lex;
use akashshell_parser::parser::parse;
use akashshell_core::evaluator::evaluate;
use akashshell_tui::EventLoop;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    // If passed a command flag (e.g. for testing/automation), execute directly and exit
    if args.len() > 2 && args[1] == "-c" {
        let input = &args[2];
        execute_line(input);
        return;
    }

    println!("🚀 Welcome to akashshell v0.1 (Elite-Grade Rust Shell)");
    println!("Type 'exit' to quit.\n");
    
    // Initialize the non-blocking UI state
    let tui = EventLoop::new();
    tui.start_render_thread();

    let stdin = io::stdin();
    let mut stdout = io::stdout();

    loop {
        print!("akashshell> ");
        stdout.flush().unwrap();

        let mut input = String::new();
        if stdin.read_line(&mut input).is_err() {
            break;
        }

        let input = input.trim();
        if input.is_empty() {
            continue;
        }

        if input == "exit" {
            break;
        }

        // Simulating the TUI keystroke handling to show off syntax highlighting logic
        for c in input.chars() {
            tui.handle_input(c);
        }
        
        let highlighted = tui.get_highlighted_input();
        println!("(Rendered internally as: {})", highlighted);

        execute_line(input);
        
        // Reset TUI state for next prompt
        if let Ok(mut state) = tui.state.lock() {
            state.current_input.clear();
            state.suggestion = None;
        }
    }
}

fn execute_line(input: &str) {
    let tokens = lex(input);
    match parse(tokens) {
        Ok(ast) => {
            match evaluate(&ast, None) {
                Ok(value) => {
                    let out = value.as_string();
                    if !out.is_empty() {
                        println!("{}", out);
                    }
                }
                Err(e) => println!("\x1b[31mError:\x1b[0m {}", e),
            }
        }
        Err(e) => println!("\x1b[31mSyntax Error:\x1b[0m {}", e),
    }
}
