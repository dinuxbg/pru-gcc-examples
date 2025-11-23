#![feature(rustc_attrs)]

#[rustc_builtin_macro]
macro_rules! asm {
    () => {}
}

fn main() {
    let ones: u32 = 0xffff;
    let zeros: u32 = 0;
    unsafe {
        asm!(
            "mov r30, {}",
            "nop",
            in(reg) ones,
        );
        asm!(
            "mov r30, {}",
            "nop",
            in(reg) zeros,
        );
    }
    loop {}
}
