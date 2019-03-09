use std::fmt::Write;
use crate::diff_tool;

//TODO: how to initialize ToolFlags without a flag??? -> remove NO_FLAG
bitflags! {
    pub struct ToolFlags: u32 {
        const NO_FLAG       = 0x0001;   
        const EXCLUDE_SAME  = 0x0002;
    }
}

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
    0xff4000,   
    0xffbf00,   
    0xffff00,   
    0xbfff00,   
    0x40ff00,   
    0x00ff80,   
    0x00bfff,   
    0x0040ff,   
    0x4000ff,   
    0x8000ff,   
    0xff00ff,   
    0xff0080,   
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
    <script>
        function hide_all_same() {{
            var elements = document.getElementsByClassName('is_all_same')

            for (var i = 0; i < elements.length; i++){{
                elements[i].style.display = 'none';
            }}
        }}
        function show_all_same() {{
            var elements = document.getElementsByClassName('is_all_same')

            for (var i = 0; i < elements.length; i++){{
                elements[i].style.display = 'table-row';
            }}
        }}
    </script>
</head>
<body>
<!-- a onclick="hide_all_same()" href='#'>hide same entries</a -->  
<!-- a onclick="show_all_same()" href='#'>show same entries</a --> 
"###);
}

fn html_post(out: &mut CustomWriter) {
    writeln!(out, r###"
</body>
</html>
"###);
}

pub fn html(diff: &diff_tool::DiffItem, out: &mut CustomWriter, toolflags: &ToolFlags) {
    html_pre(out);
    
    writeln!(out, "<table>");
    
    //write header
    writeln!(out, "    <tr>");
    writeln!(out, "        <th></th>");
    writeln!(out, "        <th></th>");
    for i_root in &diff.info().path {
        if let Some(i_root) = i_root {
            writeln!(out, "        <th style=\"writing-mode: vertical-rl;\">{}</th>", i_root.to_str().unwrap());
        } else {
            writeln!(out, "        <th></th>");
        }
    }
    writeln!(out, "    </tr>");
  
    //write directory structure
    
    for i_item in diff.test_get_iterator() {
        let duplicates = i_item.find_duplicates();
        let is_all_same = duplicates.iter().all(|x| *x == duplicates[0] );
        let mut tr_prefix = String::new();

        //NOTE: it is to slow in the browser :-(
        //if( is_all_same ) {
        //    tr_prefix += "class = \"is_all_same\"  ";
        //}

        if toolflags.contains(ToolFlags::EXCLUDE_SAME) && is_all_same { continue; }


        writeln!(out, "    <tr {}>", tr_prefix);

        writeln!(out, "    <td style=\"font-size: xx-small;\">{}</td>", i_item.relative_path().to_str().unwrap_or("???"));
        writeln!(out, "    <td style=\"text-indent: {}em;\">{}</td>", 
            i_item.depth()*2,
            i_item.name().unwrap().to_str().unwrap() );

        
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