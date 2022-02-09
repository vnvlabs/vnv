
! This defines the vnv options schema for the fortran objeect.
! Actually, whatever you put here will end up in the final report
! as the package info for the fortran pacakge. 
! @vnv_options_schema(fortran)
! {
!   "type" : "object",
!   "properties" : {
!     "length" : {"type" : "integer"}   
!    }
! }

! This is the comment that goes above it. 
! @vnv_subpackage(fortran,some_other_package)

! 

program hello
  use libvnvfortran
  use libvnv
  use iso_c_binding
  implicit none
  REAL(8) x
  INTEGER(kind=c_size_t) d0,d1
  INTEGER, DIMENSION(4) :: xx;
  REAL(4), DIMENSION(3,3) :: xy;
  TYPE(C_PTR) :: ctx;
  type(c_funptr) :: reg

  xy(1,1) = 1
  xy(2,2) = 2
  xy(3,3) = 3

  x = 12.3333

  xx(1) = 1
  xx(2) = 2
  xx(3) = 3
  xx(4) = 4
  

  
  ! This is the introduction 
  ! This will go in the introduction
  call vnv_initialize("fortran","./vnv.json", vnv_fortran_registration())
  
  ! sdfsdfsdfsdf
  ! sdfsdfsdfsdf
  ! sdfsdfsdfsdfsdfsdfsdfsdf
  ! sdfsdfsdfsdfsdfsdfsdf
  ctx = vnv_loop_init(1,"fortran","hello")
  call vnv_loop_begin(ctx)
  call vnv_loop_iter(ctx,"iter")
  call vnv_loop_end(ctx)
  
  ! sdfsdfsdfsdfsdfsdfsdfsdf
  ! sdfsdfsdfsdfsdfsdfsdfs
  ! sdfsdfsdfsdfsdfsdf
  ! sdfsdfsdfsdfsdf
  ctx = vnv_point_init(1, "fortran","hello1")
  call vnv_declare_integer_array(ctx, "1sdfsdf", xx,size(xx))
  call vnv_declare_float_matrix(ctx,"sdfsdf", xy, 3 , 3)
  call vnv_declare_float_parameter(ctx,"2sdfsdf", 1.4)
  call vnv_declare_double_parameter(ctx,"2sdfsdf", x)
  call vnv_declare_string_parameter(ctx,"2sdfsdf", "sdfsdfsdfsdf")
  call vnv_point_run(ctx)
  
  ! Vnv Provides parameter processing capabilities. To get a parameter 
  ! from the parameter set call vnv_get_***_parameter. 
  print *, "The length value is: " , vnv_get_integer_parameter("fortran","/length", 2222)

  ! A Plug will allow you to replace a certain chunk of code with an external
  ! chunk of code (written in C++ at the moment). If it is not replaced the 
  ! function returns 1 and your code will run. A plug looks like this. 
  ctx = vnv_plug_init(1, "fortran","plug_one")
  call vnv_declare_float_parameter(ctx,"2sdfsdf", 1.4)
  IF ( vnv_plug_run(ctx) .eq. 0 ) THEN
    print *,"The Plug Was not overriden"
  ENDIF

  ! A Iteration point will allow you to do a parameter optimization.
  ! If it is not replaced the 
  ! function returns 1 and your code will run. A plug looks like this. 
  ctx = vnv_iterator_init(1, "fortran","iterator_one",3)
  call vnv_declare_float_parameter(ctx,"2sdfsdf", 1.4)
  DO WHILE ( vnv_iterator_run(ctx) .ne. 0 ) 
    print *,"The Iterator is running -- You should see this message 3 times at least."
  END DO

  ! You are free to use the injection point logging interface
  call vnv_debug(1,"fortan","this is the message")
  call vnv_warning(1,"fortan","this is the message")
  call vnv_info(1,"fortan","this is the message")
  call vnv_error(1,"fortan","this is the message")

  ! You can define an input file 
  call vnv_input_file(1,"fortran","f1","./inputs.json","json")
  
  ! The comment becomes the description in the final report. Use
  ! auto to allow VNV to use the extension to guess the file reader
  call vnv_input_file(1,"fortran","f2","./inputs.text","auto")

  ! The same interface works for output files. 
  call vnv_output_file(1,"fortran","f3","./inputs.json","json")
  

  ! Finalize shuts everything down -- This should be at the end. 
  call vnv_finalize("fortran")

end program hello

