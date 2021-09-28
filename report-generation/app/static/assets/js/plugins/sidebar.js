

function remove_plugin(id_) {
  event.preventDefault()
  confirm_modal("Remove Plugin", "Are you sure. You will no longer be able to use a reader defined in this plugin. Plugins will remain active until you next reset the reader.", "Yes","No", (e,m)=>{
     if (e) {
        $( "#plugin-" + id_).remove();
        $.post( "/plugin/delete/" + id_, function( data ) {
              m.modal('hide')
        });
     }
  })
}

function add_plugin(event) {
   $('#new_plugin_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}
