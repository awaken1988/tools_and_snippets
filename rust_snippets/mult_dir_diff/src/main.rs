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
}
