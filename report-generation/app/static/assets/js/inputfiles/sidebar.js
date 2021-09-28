
function remove_inputfile(id_, event) {
  event.preventDefault()

  confirm_modal("Remove Input File", "Removing an input file will remove it from the view, but not the file system. Do you still want to remove this file? ", "Yes","No", (e,m) =>{

     if (e) {
        $( "#inputfile-" + id_).remove();
        $.post( "/inputfiles/delete/" + id_, function( data ) {
            m.modal('hide')
        });
     }
  })

  }

function add_input_file(event) {
   $('#new_inputfile_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}
