var app = angular.module('zsdn-modules', [ 'smart-table' ]);

app.controller('moduleCtrl', [ '$scope','$http', function(scope, http) {
	// store for data
	scope.rowCollection = [];
	// store for displayed data 
	scope.displayedCollection = [].concat(scope.rowCollection);
	//request data from server
	XHRUpdate();

	/**
	 * Called when button update is clicked.
	 */
	scope.update = function update() {
		XHRUpdate();
	};
	
	/**
	 * Sends XHR to server to get the data.
	 */
	function XHRUpdate() {
		http.get(url + '/RESTAdmin/rest/modules',{cache: false}).
			then(function (response){
				scope.rowCollection = response.data;
				scope.displayedCollection = [].concat(scope.rowCollection);
				//console.log("new Data received");
			}, function(response){
				// errorCallback
				//window.alert(response.message);
				console.warn("Error while calling RESTAdmin/rest/modules. Status Code: " + response.status + " " + response.statusText);
		});
	};
	
	/**
	 * Enable or disable a running module.
	 */
	scope.changeState = function changeState(row) {
		if(row.currentState == "Active") {
			// disable
			http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=disable').
				then(function(response){
					XHRUpdate();
				}, function(response){
					// errorCallback
					//window.alert(response.message);
					console.warn("Error during disable! --> Status Code: " + response.status + " " + response.statusText);
					XHRUpdate();
			});
		} else if (row.currentState == "Inactive") {
			// enable
			http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=enable').
				then(function(response){
					XHRUpdate();
				}, function(response){
					// errorCallback
					//window.alert(response.message);
					console.warn("Error during enable! --> Status Code: " + response.status + " " + response.statusText);
					XHRUpdate();
			});
		}
	};
	
	/**
	 * Stop a running module.
	 */
	scope.stopModule = function stopModule(row) { 
		http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=stop').
			then(function(response){
				XHRUpdate();
			}, function(response){
				// errorCallback
				//window.alert(response.message);
				console.warn("Error during stop! --> Status Code: " + response.status + " " + response.statusText);
				XHRUpdate();
		});
	};
	
}]);