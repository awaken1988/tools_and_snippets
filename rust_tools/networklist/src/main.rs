// https://users.rust-lang.org/t/running-powershell-cmdlets-and-functions/200/6

#[cfg_attr(windows, path = "os_specific_windows.rs")]
mod os_specific;

use std::io::{stdout, Write};
use std::fs;
use std::fmt::Write as FmtWrite;
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};
use serde_json;
use std::collections::HashSet;
use std::collections::HashMap;
use std::process::Command;
use std::ffi::OsString;
use std::ffi::OsStr;
use tempfile;

mod imtui;
use imtui::Widget;


#[derive(Debug)]
pub struct UriParsed {
    _original: String,
    proto: String,
    user: Option<String>,
    port: Option<u16>,
    host: String,
    path: String,
}

#[derive(Debug)]
pub struct NetlistItem {
    path: String,
    tags: HashSet<String>,
    name_value: HashMap<String, String>,
    parsed: UriParsed,
}

struct DrawState {
    list_line_start: u32,
    list_line_count: u32,
    list_selected: u32,

    list_data: Vec<NetlistItem>,
    status_bar: String,
}

fn main() {
    let mut config = load_config();
    let mut config_list = imtui::List::new();
    let mut cmd_output = imtui::List::new();

    cmd_output.add_row( vec!["bla".to_string()] );
    
    for i_cfg in &config {
        config_list.add_row( vec![
            format!("{}", i_cfg.parsed._original).to_string(),
            format!("{}", i_cfg.tags.iter().map(|x| &x[..]).collect::<Vec<&str>>().join(" ")  ).to_string(),
        ] );
        println!("{:?}",i_cfg.parsed);
    }

    //return;

    let mut status_bar = imtui::Label::new("...");
    let mut help_bar = imtui::Label::new("Esc: Exit;   F5: Refresh;   Enter: Mount Item;");
    let mut credits = imtui::Label::new("...");

    loop {
        {
            let term_size = terminal::size().unwrap();
            let mut layout = imtui::TableLayout::new();

            layout.add(&mut config_list, imtui::Size2D{x: 0, y: 0})
                .expand(&imtui::Size2D{x: 1, y: 3})
                .border(&imtui::Size2D{x: 1, y: 1});
            layout.add(&mut cmd_output, imtui::Size2D{x: 0, y: 1})
                .expand(&imtui::Size2D{x: 1, y: 2})
                .border(&imtui::Size2D{x: 1, y: 1}); 
            layout.add(&mut status_bar, imtui::Size2D{x: 0, y: 2}); 
            layout.add(&mut help_bar, imtui::Size2D{x: 0, y: 3}); 
            layout.add(&mut credits, imtui::Size2D{x: 1, y: 3}).border(&imtui::Size2D{x: 1, y: 1}); 

            setup_screen();
            layout.draw(imtui::Size2D{x: 0, y: 0}, imtui::Size2D{x: term_size.0 as usize, y: term_size.1 as usize});
        }
        


        let e = read().unwrap();
        match e {
            Event::Key(e) if e.code == KeyCode::Esc => {
                    return;
            }
            Event::Key(e) if e.code == KeyCode::F(5) => {
                
            }
            Event::Key(e) if e.code == KeyCode::Up => {
                config_list.decrement_selection(1);
            }
            Event::Key(e) if e.code == KeyCode::Down => {
                config_list.increment_selection(1);
            }
            Event::Key(e) if e.code == KeyCode::Enter => {
                if let Some(selection) = config_list.selection {
                    handle_item( &config[selection], &mut cmd_output );
                }
            }
            _ => {
                continue
            }
        }

        
    }
}

fn handle_item(aItem: &NetlistItem, aOutput: &mut imtui::List)
{
    let mut is_ok = false;
    let mut ret_num: i32 = -1;
    let mut output = String::new();

    match &aItem.parsed.proto[..] {
        "smb" => {
            let result = os_specific::handle_smb(aItem);
            is_ok = result.0;
            ret_num = result.1;
            output = result.2;
        }
        "ssh" => {
            let result = os_specific::handle_ssh(aItem);
            is_ok = result.0;
            ret_num = result.1;
            output = result.2;
        }
        _ => {
            return;
        }
    }

    for i_line in output.lines() {
        aOutput.add_row( vec![i_line.to_string(), ] );
    }
    aOutput.increment_selection( output.lines().count() );
}

fn load_config() -> Vec<NetlistItem> {

    let mut content = fs::read_to_string("../../../../netlist.json").unwrap();
    let content: serde_json::Value = serde_json::from_str(&content).unwrap();

    let mut ret: Vec<NetlistItem> = vec![];

    for iShare in content["netlist"].as_array().unwrap() {
        let path = iShare["path"].as_str().unwrap().to_string();
        let tags: HashSet<String> = iShare["tags"].as_str().unwrap().to_string().split(" ").map(|x| x.to_string()).collect();
        let parsed = UriParsed::from_str(&path).unwrap();

        let mut name_value = HashMap::new();

        for i_tag in &tags {
            if !i_tag.contains("=") { continue;}

            let splitted: Vec<&str> = i_tag.split("=").collect();
            if splitted.len() != 2 {
                panic!("{} contains illegal \"s\"", iShare["tags"].as_str().unwrap())
            }

            name_value.insert(splitted[0].to_string(), splitted[1].to_string());
        }

        ret.push( NetlistItem{
            path: path,
            tags: tags,
            name_value: name_value,
            parsed: parsed,
        });

        let x = UriParsed::from_str( &ret.last().unwrap().path );
        println!("{:?}", x)
    }
    
    return ret;
}

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

    stdout().queue( style::SetForegroundColor( style::Color::White ) );

    stdout().flush();
}

fn input_text(intent: &str, aHideChars: bool) -> String {
    let mut show_tui = |text: &str| {
        setup_screen();
        stdout().queue( cursor::MoveTo(4, 2 as u16) );
        stdout().queue( style::Print(intent.to_string()) );
        stdout().queue( cursor::MoveTo(4 + (intent.chars().count() as u16) + 1, 2 as u16) );
        if aHideChars {
            let fill = (0..text.chars().count()).map(|x| "*".to_string()).collect::<String>();
            stdout().queue( style::Print( fill ) );
        }
        else {
            stdout().queue( style::Print( text.to_string() ) );
        }

        stdout().flush();
    };

    let mut ret = String::new();

    show_tui("");

    loop {
        let e = read().unwrap();
        match e {
            Event::Key(e) if e.code == KeyCode::Backspace => {
                if ret.chars().count() < 1 {continue;}
                ret.pop();   
            }
            Event::Key(e) if e.code == KeyCode::Enter => {
                return ret;
            }
            Event::Key(e) => { //TODO: match KeyCode::Char(e) before {
                if let KeyCode::Char(e) = e.code {
                    ret.push( e );
                }
            }
            _ => {
                continue
            }
        }

        show_tui(&ret);
    };

    return String::new();
} 

fn remove_last_char(s: &str, s_remove: &str) -> String {
    let mut ret = s.to_string();

    for i in s_remove.chars() {
        ret.pop();
    }

    return ret;
}

fn peek_next(s: &Vec<char>, peek: &str, start_pos: usize) -> bool {
    if s.len() < 1 || (start_pos+peek.chars().count()) >= (s.len()-1) {
        return false;
    }

    for iChar in peek.char_indices() {
        if s[start_pos+iChar.0] != iChar.1 {
            return false;
        }
    }

    return true;
} 

impl UriParsed {
    fn from_str(s: &str) -> Result<UriParsed, String> {
        let PROTO_SEP = "://";
        let USER_SEP = "@";

        let mut ret = UriParsed { 
            _original: s.to_string(), 
            proto: String::new(), 
            user: None,
            port: None,
            host: String::new(),
            path: String::new(), };

        #[derive(PartialEq)]
        enum ParseState {
            EXPECT_PROTO,
            EXPECT_USER_OR_HOST,
            EXPECTED_HOST,
            EXPECTED_PORT,
            EXPECTED_PATH,
            END,
        }

        let mut last = String::new();
        let mut state = ParseState::EXPECT_PROTO;
        let ch: Vec<char> = s.chars().collect();

        let mut iChIdx = 0;
        while iChIdx < ch.len() {
            last.push(ch[iChIdx]);
            let mut addional_idx = 0;
            let is_end = iChIdx+1 == ch.len();

            if ParseState::EXPECT_PROTO == state && peek_next(&ch, PROTO_SEP, iChIdx+1) {
                ret.proto = last.clone();
                state = ParseState::EXPECT_USER_OR_HOST;
                
                last.clear();
                iChIdx += PROTO_SEP.chars().count();
            }
            else if ParseState::EXPECT_USER_OR_HOST == state && peek_next(&ch, USER_SEP, iChIdx+1) {
                ret.user = Some(last.clone());
               
                iChIdx += USER_SEP.chars().count();
                last.clear();
            } 
            else if ParseState::EXPECT_USER_OR_HOST == state || ParseState::EXPECTED_HOST == state {
                let is_ip6_start  = last.starts_with("[");
                let is_ip6_end    = last.ends_with("]");
                let is_next_dp    = peek_next(&ch, ":", iChIdx+1);
                let is_next_slash = peek_next(&ch, "/", iChIdx+1);
                let mut is_ready = false;

                if !is_ip6_start && (is_next_dp || is_next_slash) {
                    is_ready = true;
                }
                else if is_ip6_start && is_ip6_end && (is_next_dp || is_next_slash) {
                    is_ready = true;
                }
                else if is_end {
                    is_ready = true;
                }

                if is_ready {
                    if is_next_dp { 
                        state = ParseState::EXPECTED_PORT;
                    }
                    else {
                        state = ParseState::EXPECTED_PATH;
                    }
                    
                    ret.host = last.clone();
                    last.clear();
                    addional_idx = 1;
                }
            }
            else if ParseState::EXPECTED_PORT == state {
                if peek_next(&ch, "/", iChIdx+1) || is_end {
                    ret.port = if let Ok(x) = last.parse::<u16>() {
                        Some(x)
                    } else {
                        return Err(format!("port is not an integer; path={}", s));
                    };

                    state = ParseState::EXPECT_USER_OR_HOST;
                    
                    last.clear();
                    iChIdx += 1;
                }
            }
            else if ParseState::EXPECTED_PATH == state {
                if iChIdx == (ch.len()-1) {
                    ret.path = "/".to_string() + &last;
                    last.clear();
                    state = ParseState::END;
                }
            }

             iChIdx += 1+addional_idx;
        };

        return Ok(ret)
    }

    
}
