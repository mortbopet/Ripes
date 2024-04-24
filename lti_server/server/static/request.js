async function get_task_list()
{
	let response = await fetch("http://127.0.0.1:5000/tasks", {method: "GET"});
	return await response.json().body;
}

async function send_solution(str_solution, task_id)
{
	let response = await fetch("http://127.0.0.1:5000/check", {method: "POST", headers: {"Content-Type": "application/json"}, body: {user: uid, task: task_id, solution: str_solution}});
	let feedback = await response.json();
	return stringToNewUTF8(feedback.body["answer"]);
}
