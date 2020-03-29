
macro_rules! my_macro {
    ($name:ident) => {
        print!("Hello World! {} = {}", stringify!($name), $name);
    };
}

macro_rules! my_list {
    ($item:ident) => {
        print!("{} = {:?},",stringify!($item), $item);
    };

    ($item:ident, $($item1:ident), +) => {
        my_list!($item);
        my_list!($($item1),+);
    };
}

fn main() {
    let a = 10;
    let b = 11;
    let c = 12;

    //my_macro!(a);

    my_list!(a,c,b);
}



