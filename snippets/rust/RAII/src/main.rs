use scopeguard;



struct My {
    num: String,
}

impl My {
    fn print_guarded(&mut self) {
        let orig =  self.num.clone();
        let curr = &mut self.num;
        curr.push('x');
        
        let mut s0 = scopeguard::guard(self, |s| {
            s.num = orig;
        });
        My::inner0(&mut s0);



        let orig =  s0.num.clone();
        let mut s1 = scopeguard::guard(s0, |mut ss| {
            ss.num = orig;
        });
        My::inner1(&mut s1);
    }

    fn inner0(&mut self) {
        self.num.push('y');
        println!("main: {}", self.num);
    }
    fn inner1(&mut self) {
        self.num.push('z');
        println!("main: {}", self.num);
    }
}



fn main() {
    let mut my = My{num: "123".to_string()};

    my.print_guarded();
    my.print_guarded();
}
