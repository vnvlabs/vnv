
function drawGraph(graph, id) {
    var svg = d3.select("#" + id),
    width = +svg.attr("width"),
    height = +svg.attr("height"),
    WORLD_SIZE= svg.attr("worldsize"),
    filename=svg.attr("filename");	
    
    var color = d3.scaleOrdinal(d3.schemeCategory20);
    var rScale = d3.scaleLinear().domain([0, WORLD_SIZE]).range([10,50]);
    var xScale = d3.scaleLinear().domain([0, WORLD_SIZE-1]).range([0, width]);

    var simulation = d3.forceSimulation()
	.force("link", d3.forceLink().id(function(d) { return d.id; }).distance(100))
    .force("charge", d3.forceManyBody().strength(-100))
    .force("center", d3.forceCenter(width / 2, height / 2))
    .force('collision', d3.forceCollide().radius(function(d) {
         return rScale(d.group);
     }))
    .force('x', d3.forceX().x(function(d) {
        return xScale( WORLD_SIZE - d.group );
     }))


  var link = svg.append("g")
      .attr("class", "links")
    .selectAll("line")
    .data(graph.links)
    .enter().append("line")
      .attr("stroke-width",  4);

  var node = svg.append("g")
      .attr("class", "nodes")
    .selectAll("g")
    .data(graph.nodes)
    .enter().append("g")
    
  var circles = node.append("circle")
      .attr("r", function(d) { return rScale(d.group); } )
      .attr("fill", function(d) { return color(d.group); })
      .call(d3.drag()
          .on("start", dragstarted)
          .on("drag", dragged)
          .on("end", dragended))
       .on("click",function(d) {
	   pathway = []
	   buildPathway(pathway, graph, d);
	   circles.attr("fill", function(dd) { 
		if (pathway.includes(dd.id)) {
		   return "red";
		} 
		 return color(dd.group)
	   })
	   link.style("stroke", function(dd) {
               if (pathway.includes(dd.target.id) && pathway.includes(dd.source.id)) {
                   return "red"
	       } 
	       return "rgb(153,153,153)"	       
	   });
	   document.querySelectorAll("[vnvcomm]").forEach(function(t){
                t.hidden = ! pathway.includes(t.getAttribute("vnvcomm")) 
	   });

    	})
	.on("dblclick",function(d) {
	   circles.attr("fill", function(dd) { 
	       return color(dd.group)
	   })
	   link.style("stroke", function(dd) {
	       return "rgb(153,153,153)"	       
	   });
	   document.querySelectorAll("[vnvcomm]").forEach(function(t){
                t.hidden = false 
	   });

    	});
        

        

  var lables = node.append("text")
      .text(function(d) {
	 if (d.group === parseInt(WORLD_SIZE,10)) {
		return "W"
	 }
	 if ("world-rank" in d) {
             return d["world-rank"];
	 }
	 return d.id;
      })
      .attr('x', 0)
      .attr('y', 0);

  node.append("title")
      .text(function(d) { return d.id; });

  simulation
      .nodes(graph.nodes)
      .on("tick", ticked);

  simulation.force("link")
      .links(graph.links);

  function ticked() {
    link
        .attr("x1", function(d) { return d.source.x; })
        .attr("y1", function(d) { return d.source.y; })
        .attr("x2", function(d) { return d.target.x; })
        .attr("y2", function(d) { return d.target.y; });

    node
        .attr("transform", function(d) {
          return "translate(" + d.x + "," + d.y + ")";
        })
  }

function buildPathway(pathway,graph,d){
   pathway.push(d.id);
   d.parents.forEach(element => buildPathway(pathway, graph, graph.nodes.find(par => par.id == element)))
   return pathway
}

function dragstarted(d) {
  if (!d3.event.active) simulation.alphaTarget(0.3).restart();
  d.fx = d.x;
  d.fy = d.y;
}

function dragged(d) {
  d.fx = d3.event.x;
  d.fy = d3.event.y;
}

function dragended(d) {
  if (!d3.event.active) simulation.alphaTarget(0);
  d.fx = null;
  d.fy = null;
}

function clicked(d) {
  alert("sdfsdf")
}

}
//drawGraph("#commMap")

