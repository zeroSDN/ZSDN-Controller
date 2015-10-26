var app = angular.module('zsdn-modules', [ 'smart-table' ]);

app.controller('moduleCtrl', [ '$scope','$http', function(scope, http) {

	scope.rowCollection = [];
	
	scope.displayedCollection = [].concat(scope.rowCollection);
	XHRUpdate();

	scope.update = function update() {
		XHRUpdate();
	};
	
	function XHRUpdate() {
		http.get(url + '/RESTAdmin/rest/modules' + '?' + Math.floor(Math.random() * 1000)).
		then(function(response){
			scope.rowCollection = response.data;
			scope.displayedCollection = [].concat(scope.rowCollection);
			console.log("new Data received");
		}, function(response){
			console.warn("Error while calling RESTAdmin/rest/modules. Status Code: " + response.status + " " + response.statusText);
		});
	}
	
	scope.changeState = function changeState(row) {
		if(row.currentState == "Active") {
			// disable
			http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=disable').
			then(function(response){
				XHRUpdate();
			}, function(response){
				console.warn("Error during disable! --> Status Code: " + response.status + " " + response.statusText);
			});
		} else if (row.currentState == "Inactive") {
			// enable
			http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=enable').
			then(function(response){
				XHRUpdate();
			}, function(response){
				console.warn("Error during enable! --> Status Code: " + response.status + " " + response.statusText);
			});
		}
	}
	
	scope.stopModule = function stopModule(row) { 
		http.put(url + '/RESTAdmin/rest/modules/control?id='+ row.UniqueId + '&action=stop').
		then(function(response){
			XHRUpdate();
		}, function(response){
			console.warn("Error during stop! --> Status Code: " + response.status + " " + response.statusText);
		});
	}
	
} ]);