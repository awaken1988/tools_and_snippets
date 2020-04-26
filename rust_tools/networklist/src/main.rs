// https://users.rust-lang.org/t/running-powershell-cmdlets-and-functions/200/6

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



#[derive(Debug)]
struct UriParsed {
    _original: String,
    proto: String,
    user: Option<String>,
    port: Option<u16>,
    host: String,
    path: String,
}

#[derive(Debug)]
struct NetlistItem {
    path: String,
    tags: HashSet<String>,
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
    let mut draw_state = DrawState{
        list_line_start: 0, 
        list_line_count: 0, 
        list_selected: 0,
        list_data: load_config(),
        status_bar: String::new(),
    };

    {
        Command::new("powershell.exe")
            .arg("start-process")
                .arg("-FilePath")
                    .arg("powershell.exe")
                .arg("-ArgumentList")
                    .arg("\"-NoExit -Command new-psdrive -name k -persist -PsProvider FileSystem -Root \\\\saturn.local\\xc3po_ro \"")
                    
                      
            //    .arg("new-psdrive")
            //        .arg("-Name").arg("K")
            //        .arg("-PSProvider").arg("FileSystem")
            //        .arg("-Root").arg(r#"\\jupiter.local\xc3po_ro"#)
            //        .arg("-Persist")
            .output();
    }
    panic!("bla");



    redraw(&mut draw_state);

    loop {
        draw_state.status_bar.clear();

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
                if draw_state.list_selected >= ((draw_state.list_data.len() as u32)-1) { continue; }
                draw_state.list_selected += 1;
            }
            Event::Key(e) if e.code == KeyCode::Enter => {
                draw_state.status_bar.clear();
                handle_item( &mut draw_state );
            }
            _ => {
                continue
            }
        }

        redraw(&mut draw_state);
    }
}

fn handle_item(aDrawState: &mut DrawState)
{
    let item = &aDrawState.list_data[aDrawState.list_selected as usize];

    match &item.parsed.proto[..] {
        "smb" => {
            if let Some(user) = &item.parsed.user {
                //let password = input_text( aDrawState, "password", true );
            }

            if let Ok(result) = Command::new("ps").output() {
                let out = String::from_utf8(result.stdout).unwrap();
            }

            panic!("bla");
        }
        _ => {
            return;
        }
    }
}

fn redraw(aDrawState: &mut DrawState)
{
    setup_screen(aDrawState);

    let mut filtered_count = 0;
    let mut filtered_selected: Option<u32> = None ;
    for (iNum, iItem) in aDrawState.list_data.iter().enumerate() {
        //TODO: apply filter
        filtered_count+=1;

        if (iNum as u32) == aDrawState.list_selected {
            filtered_selected = Some(iNum as u32);
        }
    }

    let filtered_selected: u32 = if let Some(x) = filtered_selected {
        x
    }
    else {
        0
    };

    
    let mut filtered_skip: u32 = 0;
    if filtered_selected > (aDrawState.list_line_count/2) {
        filtered_skip = filtered_selected - (aDrawState.list_line_count/2);
    }

    let mut curr_filtered: i32 = -1;
    let mut line = String::new();
    for iItem in &aDrawState.list_data {
        //TODO: apply filter

        curr_filtered+=1;
        if (curr_filtered as u32) < (filtered_skip as u32) {
            continue;
        }

        let line_pos = (curr_filtered as u32 - filtered_skip as u32) as u16;

        line.clear();
        write!(line, "    |-{}", iItem.path);

        //write!(line, "[");
        //for iTag in &iItem.tags {
        //    write!(line, "{} ", iTag);
        //}
        //write!(line, "]");

        if (curr_filtered as u32) == (filtered_selected as u32) {
            stdout().queue( style::SetForegroundColor( style::Color::Green ) );
        }
        else {
            stdout().queue( style::SetForegroundColor( style::Color::White ) );
        }
        
        stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
        stdout().queue( cursor::MoveTo(0, line_pos as u16  ) );
        stdout().queue( style::Print(&line) );
        stdout().flush();

        
    }
}

fn load_config() -> Vec<NetlistItem> {

    let mut content = fs::read_to_string("../../../../netlist.json").unwrap();
    let content: serde_json::Value = serde_json::from_str(&content).unwrap();

    let mut ret: Vec<NetlistItem> = vec![];

    for iShare in content["netlist"].as_array().unwrap() {
        let path = iShare["path"].as_str().unwrap().to_string();
        let tags = iShare["tags"].as_str().unwrap().to_string().split(" ").map(|x| x.to_string()).collect();
        let parsed = if let Ok(x) = UriParsed::from_str(&path) { x } else { panic!("path=\"{}\" invalid"); };

        ret.push( NetlistItem{
            path: path,
            tags: tags,
            parsed: parsed,
        });

        let x = UriParsed::from_str( &ret.last().unwrap().path );
        println!("{:?}", x)
    }
    
    return ret;
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

    //status bar
    {
        stdout().queue( style::SetBackgroundColor( style::Color::White ) );
        stdout().queue( style::SetForegroundColor( style::Color::Black ) );

        let pos = (term_size.1 as u32) - 3;

        stdout().queue( cursor::MoveTo(0, pos as u16) );
        stdout().queue( style::Print(aDrawState.status_bar.clone()) );

    }

    aDrawState.list_line_start = 2;
    aDrawState.list_line_count = (term_size.1 as u32) -  aDrawState.list_line_start - 5 ;

    stdout().flush();
}

fn input_text(aDrawState: &mut DrawState, intent: &str, aHideChars: bool) -> String {
    let mut show_tui = |text: &str| {
        setup_screen(aDrawState);
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
    fn from_str(s: &str) -> Result<UriParsed, &'static str> {
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
                if peek_next(&ch, "/", iChIdx+1) {
                    ret.port = if let Ok(x) = last.parse::<u16>() {
                        Some(x)
                    } else {
                        return Err("port is not an integer")
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
