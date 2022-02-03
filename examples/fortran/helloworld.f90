
program hello
  use libvnv
  use iso_c_binding
  implicit none
  REAL(8) x
  INTEGER(kind=c_size_t) d0,d1
  INTEGER, DIMENSION(4) :: xx;
  REAL(4), DIMENSION(3,3) :: xy;
  TYPE(C_PTR) :: ctx;

  xy(1,1) = 1
  xy(2,2) = 2
  xy(3,3) = 3

  x = 12.3333

  xx(1) = 1
  xx(2) = 2
  xx(3) = 3
  xx(4) = 4

  call vnv_initialize("fortran","./vv-input.json")
  
  ctx = vnv_loop_init("fortran","hello")
  call vnv_loop_begin(ctx)
  call vnv_loop_iter(ctx,"iter")
  call vnv_loop_end(ctx)
  
  ctx = vnv_point_init("fortran","hello1")
  call vnv_declare_integer_array(ctx, "1sdfsdf", xx,size(xx,kind=c_size_t))
  call vnv_declare_float_matrix(ctx,"sdfsdf", xy, INT8(3) , INT8(3))
  call vnv_declare_float_parameter(ctx,"2sdfsdf", 1.4)
  call vnv_declare_double_parameter(ctx,"2sdfsdf", x)
  call vnv_declare_string_parameter(ctx,"2sdfsdf", "sdfsdfsdfsdf")
  call vnv_point_run(ctx)
  
  print*,vnv_get_string_parameter("sdfsdf","sdfsdf","2222")

  ctx = vnv_plug_init("fortran","plug_one")
  call vnv_declare_float_parameter(ctx,"2sdfsdf", 1.4)
  IF ( vnv_plug_run(ctx) .ne. 0 ) THEN
    print *,"SDFSD"
  ENDIF


  
  call vnv_finalize("fortran")

end program hello

