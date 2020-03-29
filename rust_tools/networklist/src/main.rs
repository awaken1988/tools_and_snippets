use std::io::{stdout, Write};
use std::fs;
use std::fmt::Write as FmtWrite;
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};
use serde_json;

struct DrawState {
    list_offset: u16,
    list_maxlen: u16,
    list_selected: u16,
}

fn main() {
    let mut draw_state = DrawState{list_offset: 0, list_maxlen: 0, list_selected: 0};

    redraw(&mut draw_state);

    loop {
        let e = read().unwrap();
        match e {
            Event::Key(e) if e.code == KeyCode::Esc => {
                    return;
            }
            Event::Key(e) if e.code == KeyCode::F(5) => {
                
            }
            Event::Key(e) if e.code == KeyCode::Up => {
                if draw_state.list_selected < 1 { continue; }
                draw_state.list_selected -= 1;
            }
            Event::Key(e) if e.code == KeyCode::Down => {
                draw_state.list_selected += 1;
            }
            _ => {
                continue
            }
        }

        redraw(&mut draw_state);
    }
}

fn redraw(aDrawState: &mut DrawState)
{
    setup_screen(aDrawState);

    let mut list_idx: u16 = 0;
    let mut line = String::new();
    foreach_config_line( |path,alias| {
        line.clear();
        write!(line, "    |-{} [{}]", path, alias);

        stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
        if aDrawState.list_selected == list_idx {
            stdout().queue( style::SetForegroundColor( style::Color::Green ) );
        }
        else {
            stdout().queue( style::SetForegroundColor( style::Color::White ) );
        }
        
        stdout().queue( cursor::MoveTo(0, list_idx  ) );
        stdout().queue( style::Print(&line) );
        stdout().flush();

        list_idx = list_idx + 1;
    });
}

fn foreach_config_line<F>(mut f: F) where
    F: FnMut(&str, &str) 
{
    let mut content = fs::read_to_string("../../../../netlist.json").unwrap();
    let content: serde_json::Value = serde_json::from_str(&content).unwrap();

    for iShare in content["netlist"].as_array().unwrap() {
        f(iShare["path"].as_str().unwrap(), iShare["alias"].as_str().unwrap() );
    }
}

fn setup_screen(aDrawState: &mut DrawState) {
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

    //headline
    {
        stdout().queue( cursor::MoveTo(0, 0) );
        stdout().queue( style::SetBackgroundColor( style::Color::Green ) );
        stdout().queue( style::SetForegroundColor( style::Color::Black ) );
        stdout().queue( style::Print("netlist") );

    }

    //help
    let help = vec!["ESC: quit", "F5: quit"];
    let mut help_next_pos: usize = 0;
    for iHelp in help {
        stdout().queue( cursor::MoveTo(help_next_pos as u16, term_size.1-1) );
        stdout().queue( style::SetBackgroundColor( style::Color::Green ) );
        stdout().queue( style::SetForegroundColor( style::Color::Black ) );
        stdout().queue( style::Print(iHelp) );

        help_next_pos = help_next_pos + iHelp.chars().count() + 1;
    }
   
    aDrawState.list_offset = 2;
    aDrawState.list_maxlen = (term_size.1 -  aDrawState.list_offset - 3) as u16 ;

    stdout().flush();
}
