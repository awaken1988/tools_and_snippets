use std::ops::Deref;
use std::fmt;

#[allow(dead_code)]
struct MyComplex {
    real: i32,
    im:   u32,
}

impl AsRef<i32> for MyComplex {
    fn as_ref(&self) -> &i32 {
        return &self.real;
    }
}

impl AsRef<u32> for MyComplex {
    fn as_ref(&self) -> &u32 {
        return &self.im;
    }
}

//Only for test...deref makes only sense on e.g smartpointer...
impl Deref for MyComplex {
    type Target = i32;

    fn deref(&self) -> &Self::Target {
        return &self.real;
    }
}

impl fmt::Display for MyComplex {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({} {})", self.real, self.im)
    }
}



fn main() {

    let x = MyComplex{real: 10, im: 20};
    let real_ref: & i32 = x.as_ref();
    let im_ref          = AsRef::<u32>::as_ref(&x);

    println!("Hello, world! {} {}", real_ref, im_ref);
    println!("deref: {}", *x);
    println!("fmt::Display {}", &x);
}
