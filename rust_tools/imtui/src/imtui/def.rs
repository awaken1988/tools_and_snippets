use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};

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

    stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
    stdout().queue( style::SetForegroundColor( style::Color::White ) );
    stdout().flush();
}

#[derive(Copy, Clone, PartialEq, Debug, Default)]
pub struct Size2D {
    pub x: usize,
    pub y: usize,
}

impl Size2D {
    pub fn zero() -> Size2D {
        return Size2D {x: 0, y: 0};
    }
}

pub trait Widget {
    fn min_space(&self) -> Size2D;
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D);
    fn handle_key(&mut self, aKeyEvent: KeyEvent);
}
