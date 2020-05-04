use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

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

#[derive(Copy, Clone)]
pub struct Size2D {
    pub x: usize,
    pub y: usize,
}

pub trait Widget {
    fn min_space(&self) -> Size2D;
    fn expand(&self) -> usize;
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D);
}