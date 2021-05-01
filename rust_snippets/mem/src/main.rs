

fn main() {

    println!("Hello, world!");



    println!("usize = {}", std::mem::size_of::<String>());
    println!("usize = {}", std::mem::align_of::<String>());



    let a: u32 = 0x12345678;
    let b: u32 = 0xaabbccdd;
    let c: u32 = 0xaaaa1111;
    let my_ptr = &b as *const u32;

    println!("b address {:p}", my_ptr);

    for i in 0..25 {
        unsafe {
            let curr_ptr = my_ptr.offset(-10+i);
            println!("address={:p}; data={:x}", curr_ptr, *curr_ptr);
        }

    }



}
