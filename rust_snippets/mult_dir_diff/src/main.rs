use std::path::PathBuf;

mod diff_tool;
mod reporter;

//TODO: what exactly is &mut dyn FnMut :-O ???
//TODO: why  info: Vec<Option<Box<DirEntry>>> doesn't work

fn main() {
    //println!("Hello, world!");

    let dirs = vec![
        Some(PathBuf::from("testdata/1_left/")),
        Some(PathBuf::from("testdata/2_right/")),
        Some(PathBuf::from("testdata/3/")),
    ];

//    let dirs = vec![
//        Some(PathBuf::from("/usr/bin")),
//        Some(PathBuf::from("/bin/")),
//    ];
//
    let root = diff_tool::diff_dirs( &dirs);

//    for i in root {
//        let depth = i.depth();
//        let mut left_space = String::new();
//
//        for i in 0..depth {
//            left_space += "    ";
//        }
//
//        println!("{:?}{:?}: {:?}", left_space, i.idx.unwrap(), i.flat_data[i.idx.unwrap()]);
//    }


    let html_result = reporter::html(&root);

    print!("{}", html_result);

}
