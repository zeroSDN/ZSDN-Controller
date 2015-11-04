var app = angular.module('zsdn-logging', [ 'smart-table' ]);

app.controller('mainCtrl', [ '$scope','$http', function(scope, http) {

	scope.rowCollection = [];
	
	scope.displayedCollection = [].concat(scope.rowCollection);
	XHRUpdate();
	
	scope.update = function update() {
		console.log("update call");
		// TODO: Only load the data from last timestamp
		console.log(new Date(scope.displayedCollection[5].timeStamp).getTime());
		var startTime = new Date(scope.displayedCollection[0].timeStamp).getTime();
		var endTime = new Date().getTime();
		http.get(url + '/RESTAdmin/rest/messages/filtered?starttime=' + startTime + '&endtime=' + endTime).
		then(function(response){
			scope.rowCollection = scope.rowCollection.concat(response.data);
			scope.displayedCollection = scope.displayedCollection.concat(scope.rowCollection);
			console.log("new Data received");
		}, function(response){
			console.warn("Error while calling RESTAdmin/rest/messages. Status Code: " + response.status + " " + response.statusText);
		});
	};
	
	function XHRUpdate() {
		http.get(url + '/RESTAdmin/rest/messages' + '?' + Math.floor(Math.random() * 1000)).
		then(function(response){
			scope.rowCollection = response.data;
			scope.displayedCollection = [].concat(scope.rowCollection);
			console.log("new Data received");
		}, function(response){
			console.warn("Error while calling RESTAdmin/rest/messages. Status Code: " + response.status + " " + response.statusText);
		});
	}
	
} ]);