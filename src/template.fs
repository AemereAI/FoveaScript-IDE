//Local Ethiopian News

function register()
{
	var descriptors = "<item><do>Default</do></item>";
	sendTextPost(descriptors);
}

function func_name(descr)
{
	var app_func_name = "";
	if(descr == "Register")
	{
		app_func_name = "register";
	}
	elif(descr == "Default")
	{
		app_func_name = "search";
	}
	elif(descr == "init Default")
	{
		app_func_name = "on_search";
	}

	sendTextPost(app_func_name);
}


function on_search()
{
	var text = "<textbox><title>Keyword</title></textbox>";
	sendTextPost(text);
}



function search(Keyword)
{

