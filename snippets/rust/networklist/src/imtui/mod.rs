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

pub fn setup_screen() {
    terminal::enable_raw_mode();
    stdout().execute( terminal::Clear(terminal::ClearType::All) );

    let term_size = terminal::size().unwrap();

    //restyle terminal
    for iRow in 0..term_size.1 {
        let fill_line = (0..term_size.0).map(|_| " ").collect::<String>();  //Slow???
        stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
        stdout().queue( style::SetForegroundColor( style::Color::Black ) );
        stdout().queue( style::Print(fill_line) );
    }

    stdout().queue( style::SetForegroundColor( style::Color::White ) );

    stdout().flush();
}

#[cfg(test)]
mod tests {

    use super::def::{*};
    use super::tablelayout::{*};
    use super::list::{*};
    use super::label::{*};
    use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};

    #[derive(Default)]
    struct TestWidget {
        min: Size2D,
        expected_draw_pos: Size2D,
        expected_draw_size: Size2D,
        description: &'static str,
    }

    impl Widget for TestWidget {
        fn min_space(&self) -> Size2D {
            return self.min;
        }
        fn draw(&self, aLeftTop: Size2D, aDimension: Size2D) {
            assert_eq!( aLeftTop, self.expected_draw_pos, "left_top {}", self.description );
            assert_eq!( aDimension, self.expected_draw_size, "dimension {}", self.description );
        }

        fn handle_key(&mut self, aKeyEvent: KeyEvent) {

        }
    }

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

        layout.add(&mut widget_vec[0], super::Size2D{x: 11, y: 10});
    }

    #[test]
    fn tablelayout_draw() {
        let mut w_0_0 = TestWidget { 
            min:                Size2D {x: 20, y: 10, },
            expected_draw_pos:  Size2D {x: 0, y: 0, },
            expected_draw_size: Size2D {x: 20, y: 10, },
            description: "w_0_0",
        };
        let mut w_1_1 = TestWidget { 
            min:                Size2D {x: 5, y: 5, },
            expected_draw_pos:  Size2D {x: 20, y: 10, },
            expected_draw_size: Size2D {x: 5, y: 5, },
            description: "w_1_1",
        };
        let mut w_2_2 = TestWidget { 
            min:                Size2D {x: 1000, y: 2000, },
            expected_draw_pos:  Size2D {x: 25, y: 15, },
            expected_draw_size: Size2D {x: 1000, y: 2000, },
            description: "w_1_1",
        };

        let mut layout = TableLayout::new();

        layout.add(&mut w_0_0, Size2D{x: 0, y: 0});
        layout.add(&mut w_1_1, Size2D{x: 1, y: 1});
        layout.add(&mut w_2_2, Size2D{x: 2, y: 2});

        layout.draw(Size2D{x: 7, y: 13}, Size2D{x: 4000, y: 4000});

    }

}
