use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

const BORDER_SPACE_X: usize = 1;
const BORDER_SPACE_Y: usize = 1;

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

    stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
    stdout().queue( style::SetForegroundColor( style::Color::White ) );
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
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D);
}

//------------------------------------------
// Label
//------------------------------------------
struct Label {
    text: String,
}

impl Label {
    fn new(text: &str) -> Box<dyn Widget> {
        let ret = Label {
            text: text.to_string(), 
        };

        return Box::new(ret);
    }
}

impl Widget for Label {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.text.chars().count(), 
            y: 1 };
    }

    fn expand(&self) -> usize {
        return 0;
    } 

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {
        let mut print_copy = self.text.clone();

        if self.text.chars().count() > aDimension.x {
            for i in 0..(self.text.chars().count()-aDimension.x) {
                print_copy.pop();
            }
        }

        stdout().queue( cursor::MoveTo(aLeftTop.x as u16, aLeftTop.y as u16) );
        stdout().queue( style::Print(print_copy) );
    }
}

//------------------------------------------
// List
//------------------------------------------
struct List {
    data: Vec<Vec<String>>
}

impl List {
    fn new() -> Box<dyn Widget> {
        let mut ret = Box::new(List {
            data: Vec::new()
        });

        ret.add_row( vec!["1.1".to_string(), "1.2".to_string(), ]);
        ret.add_row( vec!["2.1".to_string(), "2.2".to_string(), ]);
        ret.add_row( vec!["3.1".to_string(), "3.2".to_string(), ]);

        return ret;
    }

    fn clear(&mut self) {
        self.data.clear();
    }

    fn add_row(&mut self, row: Vec<String>) {
        self.data.push(row);
    }

    fn column_count(&self) -> usize {
        let mut count = 0;
        for iRow in &self.data {
            if iRow.len() > count {
                count = iRow.len();
            }
        }

        return count;
    }
}

impl Widget for List {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.column_count()*10, 
            y: 3 };
    }

    fn expand(&self) -> usize {
        return 0;
    }

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {   
        let cols = self.column_count();
        let width_per_col = aDimension.x / cols;

        let mut used_rows = 0;
        for iRow in 0..(self.data.len()) {
            if used_rows >= aDimension.y {
                break;
            }

            for (iColNum, iCol) in self.data[iRow].iter().enumerate() {
                let mut print_copy = iCol.clone();
                let separator = "|".to_string();

                let width_per_col_data = width_per_col - 1;
                if iCol.chars().count() > width_per_col_data {
                    for i in 0..(iCol.chars().count()-width_per_col_data) {
                        print_copy.pop();
                    }
                }
        
                stdout().queue( cursor::MoveTo((aLeftTop.x + width_per_col*iColNum) as u16, (aLeftTop.y + iRow) as u16) );
                stdout().queue( style::Print(print_copy) );
                //stdout().queue( cursor::MoveTo((width_per_col*iColNum-1) as u16, iRow as u16) );
                //stdout().queue( style::Print(separator) );
            }

            used_rows+=1;
        }
    }
}

struct BoxLayout<'a> {
    table: Vec<Vec<Option<BoxLayoutItem<'a>>>>,

    used_max_x: Vec<usize>,
    used_max_y: Vec<usize>,

    items_x: usize,
    items_y: usize,
}

#[derive(Clone)]
struct BoxLayoutItem<'a> {
    widget: &'a Box<dyn Widget>,
    position: Size2D,
    used_size: Size2D,
    offset: Size2D,
}

impl<'a> BoxLayout<'a> {
    fn new() -> BoxLayout<'a> {
        return BoxLayout {
            table: Vec::new(),
            used_max_x: Vec::new(),
            used_max_y: Vec::new(),
            items_x: 0,
            items_y: 0,
        };
    }

    fn add(&mut self, widget: &'a Box<dyn Widget>, position: Size2D) {
        if self.items_x <= position.x {
            self.items_x = position.x+1;
        }
        if self.items_y <= position.y {
            self.items_y = position.y+1;
        }

        //fill table to the maximum cos, rows
        if self.table.len() < self.items_x {
            self.table.append( &mut vec![vec![None; 0]; self.items_x - self.table.len()] );
        }
        for iRow in &mut self.table {
            if iRow.len() < self.items_y {
                let y_diff = self.items_y - iRow.len();
                iRow.append( &mut vec![None; y_diff] );
            }
        } 


        if self.table[0].len() <= (position.y) {
            let y_diff = (position.y+1)-self.table[0].len();
            for iRow in &mut self.table {
                iRow.append( &mut vec![None; y_diff] );
            }
        }

        for iRow in &self.table {
            println!("len {}", iRow.len());
        }
        println!("___");

        let mut entry = &mut self.table[position.x][position.y];

        if let Some(x) = entry {
            panic!("use position twice");
        }

        *entry = Some(BoxLayoutItem {
            widget: widget,
            position: position,
            used_size: Size2D{x: 0, y: 0},
            offset: Size2D{x: 0, y: 0} });
    }

    fn calc_layout(&mut self) {

        self.used_max_x = vec![0; self.table.len()];
        self.used_max_y = vec![0; self.table[0].len()];

        //set all widget to min size; all hor/vert cells should have the same size
        for iRow in 0..self.table.len() {
            for iCol in 0..self.table[iRow].len() {
                if let Some(col) = &mut self.table[iRow][iCol] {
                    col.used_size = col.widget.min_space();
                    col.used_size.x += BORDER_SPACE_X;
                    col.used_size.y += BORDER_SPACE_Y;

                    if col.used_size.x > self.used_max_x[iCol] {
                        self.used_max_x[iCol] = col.used_size.x;
                    }
                    if col.used_size.y > self.used_max_y[iRow] {
                        self.used_max_y[iRow] = col.used_size.y;
                    }
                }
            }
        }

        //calculate the offset
        for iRow in 0..self.table.len() {
            for iCol in 0..self.table[iRow].len() {
                if let Some(col) = &mut self.table[iRow][iCol] {
                    let mut offset = Size2D{x: 0, y: 0};

                    for i in 0..iCol {
                        offset.x += self.used_max_x[i];
                    }
                    for i in 0..iRow {
                        offset.y += self.used_max_y[i];
                    }

                    col.offset = offset;
                }
            }
        }
    }

    fn draw(&mut self) {
        self.calc_layout();

        for iRow in 0..self.table.len() {
            for iCol in 0..self.table[iRow].len() {
                if let Some(col) = &mut self.table[iRow][iCol] {
                    
                    //draw content
                    {
                        let mut content_size = col.used_size;
                        content_size.x -= BORDER_SPACE_X;
                        content_size.x -= BORDER_SPACE_Y;
                        col.widget.draw(col.offset, col.used_size);
                    }

                    //draw vertical bordder
                    {
                        let offset_x = col.offset.x + self.used_max_x[iCol] - 1;
                        let start_y  = col.offset.y;

                        for iRow in 0..(self.used_max_y[iRow]) {
                            stdout().queue( cursor::MoveTo(offset_x as u16, (start_y + iRow) as u16) );
                            stdout().queue( style::Print("|".to_string()) );
                        }
                    }

                    //draw horizontal border
                    {
                    let offset_y = col.offset.y + self.used_max_y[iRow] - 1;
                    let start_x  = col.offset.x;

                    for iCol in 0..(self.used_max_x[iCol]) {
                        stdout().queue( cursor::MoveTo((start_x + iCol) as u16, offset_y as u16) );
                        stdout().queue( style::Print("-".to_string()) );
                    }
                }

                }
            }
        }

        stdout().flush();
    }
}

fn main() -> () {
    setup_screen();

    let mut lbl_0_0 = Label::new(".x=0; y=0.......");
    let mut lbl_1_1 = Label::new("_x=1; y=1_");
    let mut lbl_2_2 = Label::new("?x=2; y=2?");

    let mut list_0_2 = List::new();
    let mut list_2_0 = List::new();
    let mut list_1_0 = List::new();
   

    let mut layout = BoxLayout::new(); 
    layout.add(&lbl_0_0, Size2D{x:0, y: 0});
    layout.add(&lbl_1_1, Size2D{x:1, y: 1});
    layout.add(&lbl_2_2, Size2D{x:2, y: 2});

    layout.add(&list_0_2, Size2D{x:0, y: 2});
    layout.add(&list_2_0, Size2D{x:2, y: 0});
    layout.add(&list_1_0, Size2D{x:1, y: 0});

    layout.draw();

    stdout().queue( cursor::MoveTo(80 as u16, 30 as u16) );
    stdout().flush();
}
