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

    use super::def::{*};
    use super::tablelayout::{*};
    use super::list::{*};
    use super::label::{*};

    #[test]
    fn tablelayout() {
        let mut layout = super::TableLayout::new();
        let mut widget0 = super::Label::new("Some Text");
        let mut widget1 = super::Label::new("Some Text");
        let mut widget2 = super::Label::new("Some Text");
    
        layout.add(&mut widget0, super::Size2D{x:0, y:0});
        assert_eq!( layout.is_cell_free(super::Size2D{x:0, y:0}), false );
        assert_eq!( layout.size(), super::Size2D{x: 1, y: 1} );

        layout.add(&mut widget1, super::Size2D{x:1000, y:0});
        assert_eq!( layout.is_cell_free(super::Size2D{x:0, y:0}), false );
        assert_eq!( layout.size(), super::Size2D{x: 1001, y: 1} );

        layout.add(&mut widget2, super::Size2D{x:0, y:999});
        assert_eq!( layout.is_cell_free(super::Size2D{x:0, y:0}), false );
        assert_eq!( layout.size(), super::Size2D{x: 1001, y: 1000} );

        let mut widget_vec = vec![super::Label::new("aaaa"), super::Label::new("bbbb"), super::Label::new("bbbb")];

        layout.add(&mut widget_vec[0], super::Size2D{x: 10, y: 10});
    }

}
