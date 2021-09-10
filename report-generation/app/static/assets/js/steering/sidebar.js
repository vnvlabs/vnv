
function remove_steering(id_, event) {
  event.preventDefault()

  confirm_modal("Remove Steering Application", "Removing this will remove it.  Do you still want to remove this file? ", "Yes","No", e=>{

     if (e) {
        $( "#steering-" + id_).remove();
        $.post( "/steering/delete/" + id_, function( data ) {});
     }
  })

  }

function add_steering_file(event) {
   $('#new_steering_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}
