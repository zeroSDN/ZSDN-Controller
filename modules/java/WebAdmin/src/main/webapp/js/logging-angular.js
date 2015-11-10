var app = angular.module('zsdn-logging', [ 'smart-table' ]);

app.controller('mainCtrl', [ '$scope','$http', function(scope, http) {

	// store for data
	scope.rowCollection = [];
	// store for displayed data 
	scope.displayedCollection = [].concat(scope.rowCollection);
	// request data from server
	XHRUpdate();
	
	/**
	 * Called when button update is clicked. \n
	 * Request only new data from server.
	 */
	scope.update = function update() {
		var startTime = new Date(scope.displayedCollection[0].timeStamp).getTime();
		var endTime = new Date().getTime();
		http.get(url + '/messages/filtered?starttime=' + startTime + '&endtime=' + endTime).
			then(function(response){
				scope.rowCollection = scope.rowCollection.concat(response.data);
				scope.displayedCollection = scope.displayedCollection.concat(scope.rowCollection);
				//console.log("new Data received");
			}, function(response){
				// errorCallback
				window.alert("Cassandra connect failed! Please verify that Cassandra is running and configured");
				//window.alert(response.message);
				console.warn("Error while calling rest/messages. Status Code: " + response.status + " " + response.statusText);
		});
	};
	
	/**
	 * Sends XHR to server to get the data.
	 */
	function XHRUpdate() {
		http.get(url + '/messages', {cache: false}).
			then(function(response){
				scope.rowCollection = response.data;
				scope.displayedCollection = [].concat(scope.rowCollection);
				//console.log("new Data received");
			}, function(response){
				// errorCallback
				window.alert("Cassandra connect failed! Please verify that Cassandra is running and configured");
				//window.alert(response.message);
				console.warn("Error while calling rest/messages. Status Code: " + response.status + " " + response.statusText);
		});
	};
	
}]);