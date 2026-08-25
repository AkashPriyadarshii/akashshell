pub mod conpty {
    use std::process::{Command, Stdio};
    use std::io::{self, Read};

    /// Represents a Pseudo-Terminal wrapper for Windows.
    pub struct ConPty {
        // Note: In the final production version, this would hold the HPCON handle 
        // generated from `CreatePseudoConsole`. For the v0.1 baseline, we intercept 
        // standard I/O securely to prevent legacy EXEs from corrupting the TUI.
        command_name: String,
        args: Vec<String>,
    }

    impl ConPty {
        pub fn new(command_name: &str, args: &[String]) -> Self {
            Self {
                command_name: command_name.to_string(),
                args: args.to_vec(),
            }
        }

        /// Spawns the process inside the PTY bridge and captures its output.
        pub fn spawn_and_capture(&self) -> Result<String, io::Error> {
            let mut child = Command::new(&self.command_name)
                .args(&self.args)
                .stdout(Stdio::piped())
                .stderr(Stdio::piped())
                .spawn()?;

            let mut output = String::new();
            if let Some(mut stdout) = child.stdout.take() {
                stdout.read_to_string(&mut output)?;
            }
            
            let status = child.wait()?;
            if !status.success() {
                if let Some(mut stderr) = child.stderr.take() {
                    let mut err_out = String::new();
                    stderr.read_to_string(&mut err_out)?;
                    return Err(io::Error::new(io::ErrorKind::Other, err_out));
                }
            }

            Ok(output)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::conpty::ConPty;

    #[test]
    #[cfg(target_os = "windows")]
    fn test_external_command_pty_bridge() {
        let pty = ConPty::new("cmd.exe", &["/c".to_string(), "echo pty-test".to_string()]);
        let output = pty.spawn_and_capture().unwrap();
        assert!(output.contains("pty-test"));
    }
}
