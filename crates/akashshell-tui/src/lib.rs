use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

/// Holds the synchronized state for the TUI to prevent freezing
pub struct RenderState {
    pub current_input: String,
    pub suggestion: Option<String>,
    pub history: Vec<String>,
    pub is_executing: bool,
}

pub struct EventLoop {
    pub state: Arc<Mutex<RenderState>>,
}

impl EventLoop {
    pub fn new() -> Self {
        Self {
            state: Arc::new(Mutex::new(RenderState {
                current_input: String::new(),
                suggestion: None,
                history: vec!["ls -la".to_string(), "cargo test".to_string()], // Mock local history
                is_executing: false,
            })),
        }
    }

    /// Simulates the non-blocking rendering thread that runs completely isolated
    /// from the execution Evaluator.
    pub fn start_render_thread(&self) {
        let state_clone = Arc::clone(&self.state);
        thread::spawn(move || {
            loop {
                if let Ok(state) = state_clone.lock() {
                    if state.is_executing {
                        break;
                    }
                    // Normally this is where we would flush ANSI codes to stdout via Crossterm
                }
                thread::sleep(Duration::from_millis(16)); // ~60 FPS terminal rendering
            }
        });
    }

    /// Handles raw character input, providing Fish-style auto-suggestions
    pub fn handle_input(&self, c: char) {
        if let Ok(mut state) = self.state.lock() {
            state.current_input.push(c);
            
            // Fish-style auto-suggestion logic
            state.suggestion = state.history.iter()
                .find(|h| h.starts_with(&state.current_input) && h.len() > state.current_input.len())
                .map(|h| h[state.current_input.len()..].to_string());
        }
    }
    
    /// Syntax highlights the current input strictly in memory
    pub fn get_highlighted_input(&self) -> String {
        if let Ok(state) = self.state.lock() {
            let words: Vec<&str> = state.current_input.split_whitespace().collect();
            if words.is_empty() {
                return String::new();
            }
            
            // Command is Green (32), Arguments are Cyan (36)
            let mut highlighted = format!("\x1b[32m{}\x1b[0m", words[0]); 
            for arg in &words[1..] {
                highlighted.push_str(&format!(" \x1b[36m{}\x1b[0m", arg)); 
            }
            highlighted
        } else {
            String::new()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_fish_style_suggestions() {
        let event_loop = EventLoop::new();
        event_loop.handle_input('l');
        event_loop.handle_input('s');
        
        if let Ok(state) = event_loop.state.lock() {
            // Input is "ls", history has "ls -la", suggestion should be " -la"
            assert_eq!(state.suggestion, Some(" -la".to_string()));
        }
    }
    
    #[test]
    fn test_syntax_highlighting() {
        let event_loop = EventLoop::new();
        event_loop.handle_input('l');
        event_loop.handle_input('s');
        event_loop.handle_input(' ');
        event_loop.handle_input('-');
        event_loop.handle_input('l');
        event_loop.handle_input('a');
        
        let highlighted = event_loop.get_highlighted_input();
        assert!(highlighted.contains("\x1b[32mls\x1b[0m"));
        assert!(highlighted.contains("\x1b[36m-la\x1b[0m"));
    }
}
