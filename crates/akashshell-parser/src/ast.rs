#[derive(Debug, PartialEq)]
pub enum Expr {
    Command {
        name: String,
        args: Vec<String>,
    },
    Pipeline(Vec<Expr>),
}
