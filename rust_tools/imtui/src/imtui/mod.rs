use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

mod def;
mod tablelayout;
mod list;
mod label;

pub use def::{*};
pub use tablelayout::{*};
pub use list::{*};
pub use label::{*};

#[cfg(test)]
mod tests {

    #[test]
    fn tablelayout() {
        let mut layout = super::TableLayout::new();
        let mut widget = super::Label::new("Some Text");

        layout.add(&mut widget, super::Size2D{x:0, y:0});

        assert_eq!( layout.is_cell_free(super::Size2D{x:0, y:0}), false );
    }

}
