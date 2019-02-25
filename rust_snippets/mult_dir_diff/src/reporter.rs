use std::fmt::Write;
use crate::diff_tool;

pub struct CustomWriter {
    outfile: Option<std::fs::File>,
}

impl CustomWriter {
    pub fn new() -> CustomWriter {
        return CustomWriter{ outfile: None };
    }

    pub fn set_out_file(&mut self, mut outfile: std::fs::File) {
        self.outfile = Some(outfile);
    }
}

impl std::fmt::Write for CustomWriter {
    
    fn write_str(&mut self, s: &str) -> Result<(), std::fmt::Error> {
        if let Some(outfile) = self.outfile.as_mut() {
            std::io::Write::write_all(outfile, s.as_bytes());
        } else {
            print!("{}", s);
        }
        Ok(())
    }

    fn write_char(&mut self, c: char) -> Result<(), std::fmt::Error> {
        if let Some(outfile) = &self.outfile {

        } else {
            print!("{}", c );
        }
       
        Ok(())
    }
}



//--------------------------------------------------
// html output
//--------------------------------------------------
static HTML_COMP_COLORS: &'static [i32] = &[    
    0x9A6840,
    0xCD852D,
    0x58180B,
    0x395E75,
    0xAED2DB,
];

fn html_pre(out: &mut CustomWriter) {
    writeln!(out, r###"<!doctype html>

<head>
    <meta charset="utf-8">
    <style>
        table {{
            border: 1px solid black;
        }}

        td {{
            border-bottom: 1px solid black;
        }}
    </style>
</head>
<body>
"###);
}

fn html_post(out: &mut CustomWriter) {
    writeln!(out, r###"
</body>
</html>
"###);
}

pub fn html(diff: &diff_tool::DiffItem, out: &mut CustomWriter) {
    html_pre(out);
    
    writeln!(out, "<table>");
    
    //write header
    writeln!(out, "    <tr>");
    writeln!(out, "        <th></th>");
    for i_root in &diff.info().path {
        if let Some(i_root) = i_root {
            writeln!(out, "        <th>{}</th>", i_root.to_str().unwrap());
        } else {
            writeln!(out, "        <th></th>");
        }
    }
    writeln!(out, "    </tr>");
  
    //write directory structure
    
    for i_item in diff.test_get_iterator() {
        

        writeln!(out, "    <tr>");
        writeln!(out, "    <td style=\"text-indent: {}em;\">{}</td>", 
            i_item.depth()*2,
            i_item.info_name().unwrap().to_str().unwrap() );

        for i_dup in i_item.find_duplicates().iter() {
            if let Some(i_dup) = i_dup {
                writeln!(out, "        <td style=\"background-color: #{:x};\">{}</td>", HTML_COMP_COLORS[*i_dup], i_dup);
            } else {
                writeln!(out, "        <td style=\"background-color: black;\"></td>");
            }
        }
        writeln!(out, "    </tr>");
    }
    writeln!(out, "</table>");

    html_post(out);
}