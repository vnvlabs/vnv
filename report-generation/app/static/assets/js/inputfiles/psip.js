

var process_state = ["", "None", "Basic" , "Intermediate", "Advanced"]


var main_data;
var initial_data;
var psip_file_id

function reset_data() {
    main_data = JSON.parse(JSON.stringify(initial_data))
}

function set_psip_data(data, file) {
    initial_data = data
    psip_file_id = file
    reset_data();
$(document).ready(function() {

    chart_overall = new ApexCharts(document.getElementById("radar-overall"), options2());
    chart_selected = new ApexCharts(document.getElementById("radar-selected"), options1());
    gchart = new ApexCharts(document.querySelector("#gchart"), goals_options());

    chart_overall.render()
    change_selection(0, true);

    gchart.render();
    change_tracking_card(0)
})

}



function ptc_modal_show() {
    data = main_data["ptc"][ptc_ind]
    $('#ptc-process').val(data["process"] || "The process" )
    $('#ptc-target').val(data["target"] || "A new target")
    $('#ptc-goal-0').val(data["scores"][0] || "Goal number 0 -- your current state")
    $('#ptc-goal-1').val(data["scores"][1] || "")
    $('#ptc-goal-2').val(data["scores"][2] || "")
    $('#ptc-goal-3').val(data["scores"][3] || "")
    $('#ptc-goal-4').val(data["scores"][4] || "")
    $('#ptc-goal-5').val(data["scores"][5] || "")
    $('#edit-ptc-modal').modal('show');
}

function ptc_modal_save() {
    data = main_data["ptc"][ptc_ind]
    data["process"] = $('#ptc-process').val();
    data["target"] = $('#ptc-target').val();

    goals = []

    for (var i = 0; i < 6; i++) {
       var g0 = $('#ptc-goal-' + i).val();
       if ( g0.length > 0 ) {
          goals.push(g0)
       } else {
          break;
       }
    }
    data["scores"] = goals;
    if (data["score"] > goals.length - 1)
        data["score"] = goals.length - 1

    gchart.updateOptions(goals_options())
    change_tracking_card(ptc_ind)
    $('#edit-ptc-modal').modal('hide');
    save_state();

}

function delete_selected() {
    if ( main_data["ptc"].length > 1 ) {
        main_data["ptc"].splice(ptc_ind, 1);
        gchart.updateOptions(goals_options())
        change_tracking_card(0)
        save_state();
    } else {
        alert("You need at least one PTC for now");
    }
}

function edit_selected() {
    ptc_modal_show();
}

function new_selected() {
    main_data["ptc"].push({
        "process" : "New Process",
        "target" : "Describe the goal of this tracking card",
        "score" : 0,
        "scores" : [
               "Stage 0 Goal is your current state",
               "Stage 1 Goal is your next state",
               "Stage 2 Goal is your next next state",
               "Stage 3 Goal is your acheivment state"
        ]
    });
    gchart.updateOptions(goals_options())
    change_tracking_card(main_data["ptc"].length - 1)
    edit_selected();
    save_state();
}


//SA


var schema_data_selection = 0
var schema_process_selection = 0
var chart_selected;
var chart_overall;
var gchart;

function schema_categories(){
    return Object.keys(main_data["sa"]) // Get all the main catagories
}

function selected_categories(sel){
    if (sel === undefined) {
        sel = schema_data_selection
    }

   return Object.keys( main_data["sa"][ schema_categories()[sel]]["subgoals"])
}

function selected_data(d){

    sel = d
    if (d === undefined) {
        sel = schema_data_selection
    }

    ret = []
    var r = main_data["sa"][schema_categories()[sel]]
    cc = selected_categories(sel)
    for (var i = 0; i < cc.length; i++) {
        ret.push(r["subgoals"][cc[i]]["score"] + 1 )
    }
    return ret;
}

function schema_data() {
  var res = []
  for ( var i = 0; i < schema_categories().length; i++ ) {
      s = selected_data(i)
      count = s.reduce((a,b) => a+b, 0)
      tot = 4*s.length
      res.push( 100 * count  / parseFloat(tot) )
  }
  return res
}

function render_all() {
  chart_overall.updateOptions(options2())
  chart_selected.updateOptions(options1())
  change_process_selection(schema_process_selection)

}

function reset_state() {
   reset_data()
   render_everything();
   save_state();
   save_state();
   change_tracking_card(0)


}

function render_everything() {
   render_all();
   gchart.updateOptions(goals_options())
}


function change_state(val) {
        main_data["sa"][schema_categories()[schema_data_selection]]["subgoals"][selected_categories()[schema_process_selection]]["score"] = val;
        render_all();
        save_state();
}


function change_selection(newselection, first) {
    schema_data_selection = newselection
    if (!first) {
      chart_selected.updateOptions(options1());
    } else {
      chart_selected.render();
    }

    $('#selection_title').text(schema_categories()[schema_data_selection]);
    $('#selection_description').text(main_data["sa"][schema_categories()[schema_data_selection]]["description"]);
    change_process_selection(0)
}

function increase_state() {
    var val;
    if ( schema_data_selection === schema_categories().length - 1 ) {
        val = 0;
    } else {
        val = schema_data_selection + 1;
    }
    change_selection(val, false);
}


function decrease_state() {
    var val;
    if ( schema_data_selection === 0 ) {
        val = schema_categories().length - 1;
    } else {
        val = schema_data_selection - 1;
    }
    change_selection(val, false);
}



function increase_process_state() {
    var val;
    if ( schema_process_selection ===  selected_categories().length-1) {
        val = 0;
    } else {
        val = schema_process_selection + 1;
    }
    change_process_selection(val);
}


function decrease_process_state() {
    var val;
    if ( schema_process_selection === 0 ) {
        val = selected_categories().length-1;
    } else {
        val = schema_process_selection - 1;
    }
    change_process_selection(val);
}


function change_process_selection(sel) {
       schema_process_selection = sel


       $('.process').removeClass('selected')
       catname = schema_categories()[schema_data_selection]
       cat = main_data["sa"][catname]

       subtitle = selected_categories()[schema_process_selection]
       sub = cat["subgoals"][subtitle]
       scores = sub["scores"]

       $('#process_title').text(subtitle)
       $('#process_description').text(sub["description"])
       $('#none_desc').text(scores[0])
       $('#basic_desc').text(scores[1])
       $('#inter_desc').text(scores[2])
       $('#adv_desc').text(scores[3])

       select = sub["score"]

       if (select == 0) $('#process_none').addClass('selected')
       else if (select == 1) $('#process_basic').addClass('selected')
       else if (select == 2) $('#process_inter').addClass('selected')
       else if (select == 3) $('#process_adv').addClass('selected')
}


function change_ptc(val) {
    ptc_data["score"] = val;
    gchart.updateOptions(goals_options())
    change_tracking_card(ptc_ind)
    save_state();
}

var ptc_data;
var ptc_ind

function change_tracking_card(ind) {
    ptc_ind = ind
    ptc_data = main_data["ptc"][ind]
    $('.gcard-title').text(ptc_data["process"])
    $('.gcard-subtitle').text(ptc_data["target"])

    var list = $('#gcard-scores')
    list.empty();

    for (var i = 0; i < ptc_data["scores"].length; i++) {
        // Create the list item:
        var item = document.createElement('li');


        if ( i <= ptc_data["score"] ) {
          item.classList.add("tick");

        } else {
          item.classList.add("cross");
        }
        var x = i;

        if (psip_file_id > -1 ) {
            item.setAttribute("onclick","change_ptc(" + String(i) + ");");
            item.setAttribute("style","cursor:pointer;" );
        }

        item.appendChild(document.createTextNode(ptc_data["scores"][i]))
        list.append(item);
    }

}

function process_data(data) {
  var ddata = []
  for (var i = 0; i < data.length; i++) {
    ddata.push({
        x : data[i]["process"],
        y : data[i]["score"] + 1,
        goals : [
        {
            Name : "Goal",
            value : data[i]["scores"].length ,
            strokeWidth: 5,
            strokeColor: '#775DD0'
        }
        ]
    })
  }
  return ddata
}

 function options1() {
    return {
          series: [{
          name: 'Overall Performance',
          data:  selected_data()
        }],
          chart: {
          height: "100%",
          toolbar : { show : false },
          type: 'radar',
          events: {
             markerClick: function(event, chartContext, { seriesIndex, dataPointIndex, config}) {
                console.log("CCC")
                change_process_selection(dataPointIndex);
             }
          }
        },
        xaxis: {
          categories: selected_categories(),
          min: 0,
          max: 1
        },
         plotOptions: {
          radar: {
            size: 140,
            polygons: {
              strokeColors: '#676774',
              fill: {
                colors: ['#f8f8f8', '#fff']
              }
            }
          }
        },
        yaxis : {
          min: 0,
          max: 4,
          tickAmount : 4,
          labels: {
            formatter: function(val, i) {
              return process_state[val]
            }
          }
        },
        "markers" : {
           size : 10,
        },
    };
 }

function options2() {
  return {
          series: [{
          name: 'Overall Performance',
          data:  schema_data()
        }],
          chart: {
          height: "100%",
          toolbar : { show : false },
          type: 'radar',
          events: {
             markerClick: function(event, chartContext, { seriesIndex, dataPointIndex, config}) {
                change_selection(dataPointIndex, false);
             }
          }
        },
        yaxis: {
            min : 0,
            max : 100,
            tickAmount: 5
        },
        xaxis: {
          categories: schema_categories(),
          min: 0,
          max: 100
        },
        "markers" : {
           size : 10        },
         plotOptions: {
          radar: {
            size: 140,
            polygons: {
              strokeColors: '#676774',
              fill: {
                colors: ['#f8f8f8', '#fff']
              }
            }
          }
        },
        tooltip: {
             shared: false,
             intersect: true
        }
    };
}


function goals_options() {
  return {
       series: [{
            name: 'Current',
            data: process_data(main_data["ptc"])
       }],
       chart: {
          height: 350,
          type: 'bar',
          toolbar : { show : false },
          events: {
             dataPointSelection: function(event, chartContext, { seriesIndex, dataPointIndex, config}) {
                change_tracking_card(dataPointIndex);
             }
          }
       },
       plotOptions: {
          bar: {
            horizontal: true,
          }
       },
       colors: ['#61aae1'],
       dataLabels: {
          formatter: function(val, opt) {
            const goals =
              opt.w.config.series[opt.seriesIndex].data[opt.dataPointIndex]
                .goals

            if (goals && goals.length) {
              return `${val} / ${goals[0].value }`
            }
            return val
          }
       },
       legend: {
          show: true,
          showForSingleSeries: true,
          customLegendItems: ['Current', 'Goal'],
          markers: {
            fillColors: ['#61aae1', '#775DD0']
          }
       }
 };
}

psip_readonly = true

function toggle_psip(fileId) {
    if (!psip_readonly) {
        $.post('/inputfiles/toggle_psip/' + fileId, function(data) {
            $('#show_psip_view').toggle(data)
            if (data === "show") {
                $('#psip_enabled_btn').html("Disable PSIP")
            } else {
                $('#psip_enabled_btn').html("Enable PSIP")
            }
        })
    }
}

function edit_psip_raw() {
   if (!psip_readonly) {
        temp_main_data = JSON.stringify(main_data, null,4 )
        main_data = JSON.parse(temp_main_data)
        render_everything()
        alert("editing the self assessment is not supported yet!")
   }
}

function set_psip_readonly(val) {
    psip_readonly = val
}

function save_state() {
    if (!psip_readonly) {
        url = "/inputfiles/save_psip/"+ psip_file_id + "?data=" + encodeURIComponent(JSON.stringify(main_data))
        $.post(url,function(d,s,x) {});
    }
}
