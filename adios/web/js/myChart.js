
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
                    label: 'This week',
                    data: data,
                }]
            },
     });
}


function renderExampleLineChart(divid) {
        data = [20000, 14000, 12000, 15000, 18000, 19000, 22000];
        labels =  ["sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"];
        renderLineChart(divid, data, labels);
}



