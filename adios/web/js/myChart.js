
// This is a file containing all the cool javascript stuff that we need. 
// This will be pu it the head along with every thing else that we need

// Function to render a chart using chart.js

function renderChartBase(divId, data) {
    var ctx = document.getElementById(divId).getContext('2d');
    var myChart = new Chart(ctx, data );
}
  

function renderLineChart(divId, data, labels) {
    renderChartBase(divId,  {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'y = 2x + 3',
                    data: data,
                }]
            },
           
     });
}


function renderExampleLineChart(divid) {
        labels = [0,1,2,3,4,5,6,7,8,9,10];
        data =  [3,5,7,9,11,13,15,17,19,21,23];
        renderLineChart(divid, data, labels);
}



