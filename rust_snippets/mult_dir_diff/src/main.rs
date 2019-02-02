use std::path::PathBuf;

mod diff_tool;

//TODO: what exactly is &mut dyn FnMut :-O ???
//TODO: why  info: Vec<Option<Box<DirEntry>>> doesn't work

fn main() {
    println!("Hello, world!");

    let dirs = vec![
        Some(PathBuf::from("testdata/1_left/")),
        Some(PathBuf::from("testdata/2_right/")),
        Some(PathBuf::from("testdata/3/")),
    ];

    let mut tree = diff_tool::DiffTree{ entries: vec![]};

    diff_tool::expand_dirs( &dirs, &mut tree, None );
    diff_tool::print_tree( &tree, 0, 0 );
    diff_tool::print_tree_flat( &tree);

    let my_iter = diff_tool::get_iterator( &tree );

    for i in my_iter {
        let depth = diff_tool::get_diff_depth(&tree, i);

        let mut dept_str = String::new();
        for i in 0..(depth*4) {
            dept_str += " ";
        }

        for i_path in &i.path {
            println!("{}{:?}", dept_str, i_path);
        }
        println!("{}------------------", dept_str);

        
    }

    //println!("{:?}", tree.entries[0].child);
}
