
function remove_pipeline(id_, event) {
  event.preventDefault()

  confirm_modal("Remove Pipeline", "Removing an input file will remove it from the view, but not the file system. Do you still want to remove this file? ", "Yes","No", (e,m) =>{

     if (e) {
        $( "#pipeline-" + id_).remove();
        $.post( "/pipelines/delete/" + id_, function( data ) {
            m.modal('hide')
        });
     }
  })

  }

function add_pipeline(event) {
   $('#new_pipeline_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}
