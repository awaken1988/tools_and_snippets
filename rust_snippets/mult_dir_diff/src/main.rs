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

    let mut root = diff_tool::diff_dirs( &dirs);

    for i in root.test_get_iterator() {
        let depth = i.depth();
        let mut left_space = String::new();

        for i in 0..depth {
            left_space += "    ";
        }


        println!("{:?}{:?}: {:?}", left_space, i.idx.unwrap(), i.flat_data[i.idx.unwrap()]);
    }

    //for i in &tree.flat_data {
    //    println!("{:?}", i);
    //}

   // diff_tool::print_tree_flat( &tree);

//    diff_tool::print_tree( &tree, 0, 0 );
    

//    let my_iter = diff_tool::get_iterator( &tree );

//    for i in my_iter {
//        let depth = diff_tool::get_diff_depth(&tree, i);
//
//        let mut dept_str = String::new();
//        for i in 0..(depth*4) {
//            dept_str += " ";
//        }
//
//        for i_path in &i.path {
//            println!("{}{:?}", dept_str, i_path);
//        }
//        println!("{}------------------", dept_str);
//
//        
//    }

    //println!("{:?}", tree.entries[0].child);
}
