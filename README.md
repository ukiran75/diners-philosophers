# diners-philosophers
The implementation of dining philosophers algorithm to share resources between machines in a distributed systems environment.

•	Using UDP network protocol to communicate between the systems to know the status of the resource.

•	Using Mutex locks on the resource so that there is no deadlock conditions.

•	Using pthreads to run the the client and network programs on the same machine.

•	The machine acts as a server to serve the resource request from neighbour system and acts as client to send  a request for the resource to the neighbour system.
