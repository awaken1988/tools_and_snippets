use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

fn setup_screen() {
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

    stdout().flush();
}









#[derive(Copy, Clone)]
struct Size2D {
    x: usize,
    y: usize,
}

trait Widget {
    fn min_space(&self) -> Size2D;
    fn expand(&self) -> usize;
    fn data(&self) -> String;
}

struct Button {
    m_text: String,
}

impl Button {
    fn new(text: &str) -> Box<dyn Widget> {
        let ret = Button {
            m_text: text.to_string(), 
        };

        return Box::new(ret);
    }
}

impl Widget for Button {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.m_text.chars().count(), 
            y: 1 };
    }

    fn expand(&self) -> usize {
        return 0;
    } 

    fn data(&self) -> String {
        return self.m_text.clone();
    }
}

struct BoxLayoutItem {
    widget: Box<dyn Widget>,
    min_space: Size2D,
    expand: usize,
}

struct BoxLayout {
    dimension: Size2D,
    widgets: Vec<BoxLayoutItem>,
    map: Vec<Vec<Option<usize>>>,
}

impl BoxLayout {
    fn new(a_dimension: Size2D) -> BoxLayout {
        return BoxLayout {
            dimension: a_dimension,
            widgets: vec![],
            map: vec![vec![None; a_dimension.x]; a_dimension.y]
        }
    }

    fn add_widget(&mut self, a_widget: Box<dyn Widget>, a_pos: Size2D ) {
        if None != self.map[a_pos.x][a_pos.y] {
            panic!("double use position");
        }

        self.widgets.push(BoxLayoutItem {
            min_space: a_widget.min_space(),
            expand: a_widget.expand(),
            widget: a_widget,
        }); 

        self.map[a_pos.x][a_pos.y] = Some(self.widgets.len()-1);
    }

    fn draw(&self) {
        let mut x_start = 0;

        for i_x in 0..self.dimension.x {
            
            //determine width
            let mut max_x = 0;
            for i_y in 0..self.dimension.y {
                if let Some(x) = self.map[i_x][i_y] {
                    let curr_widget = &self.widgets[x];
                    if curr_widget.min_space.x > max_x {
                        max_x = curr_widget.min_space.x;
                    }
                }
            }

            for i_y in 0..self.dimension.y {
                if let Some(x) = self.map[i_x][i_y] {
                    let curr_widget = &self.widgets[x];
                    stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
                    stdout().queue( style::SetForegroundColor( style::Color::Green ) );
                    stdout().queue( cursor::MoveTo(x_start as u16, i_y as u16  ) );
                    stdout().queue( style::Print(curr_widget.widget.data()) );
                }
                stdout().queue( cursor::MoveTo((x_start+max_x+1) as u16, i_y as u16  ) );
                stdout().queue( style::Print("|") );
            }

            x_start += max_x + 3;
        }

        stdout().flush();
    }

    
}


fn main() -> () {
    setup_screen();

    let mut layout = BoxLayout::new(Size2D{x: 4, y: 4});

    layout.add_widget(Button::new("Button 1"), 
    Size2D{x: 0, y: 0} );
    
    layout.add_widget(Button::new("Button 2 ..."), 
    Size2D{x: 1, y: 1} );

    layout.add_widget(Button::new("Button 1"), 
    Size2D{x: 2, y: 2} );

    layout.add_widget(Button::new("Button 2 ..."), 
    Size2D{x: 3, y: 3} );

    layout.add_widget(Button::new("Button 1"), 
    Size2D{x: 1, y: 0} );

    layout.add_widget(Button::new("Button 2 ............"), 
    Size2D{x: 2, y: 1} );

    layout.add_widget(Button::new("1228383884485959540403030399"), 
    Size2D{x: 0, y: 3} );

    layout.draw();
}
