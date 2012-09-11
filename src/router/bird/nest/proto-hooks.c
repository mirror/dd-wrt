/*
 *	BIRD -- Documentation for Protocol Hooks (dummy source file)
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Protocol hooks
 *
 * Each protocol can provide a rich set of hook functions referred to by pointers
 * in either the &proto or &protocol structure. They are called by the core whenever
 * it wants the protocol to perform some action or to notify the protocol about
 * any change of its environment. All of the hooks can be set to %NULL which means
 * to ignore the change or to take a default action.
 */

/**
 * preconfig - protocol preconfiguration
 * @p: a routing protocol
 * @c: new configuration
 *
 * The preconfig() hook is called before parsing of a new configuration.
 */
void preconfig(struct protocol *p, struct config *c)
{ DUMMY; }

/**
 * postconfig - instance post-configuration
 * @c: instance configuration
 *
 * The postconfig() hook is called for each configured instance after
 * parsing of the new configuration is finished.
 */
void postconfig(struct proto_config *c)
{ DUMMY; }

/**
 * init - initialize an instance
 * @c: instance configuration
 *
 * The init() hook is called by the core to create a protocol instance
 * according to supplied protocol configuration.
 *
 * Result: a pointer to the instance created
 */
struct proto *init(struct proto_config *c)
{ DUMMY; }

/**
 * reconfigure - request instance reconfiguration
 * @p: an instance
 * @c: new configuration
 *
 * The core calls the reconfigure() hook whenever it wants to ask the
 * protocol for switching to a new configuration. If the reconfiguration
 * is possible, the hook returns 1. Otherwise, it returns 0 and the core
 * will shut down the instance and start a new one with the new configuration.
 *
 * After the protocol confirms reconfiguration, it must no longer keep any
 * references to the old configuration since the memory it's stored in can
 * be re-used at any time.
 */
int reconfigure(struct proto *p, struct proto_config *c)
{ DUMMY; }

/**
 * dump - dump protocol state
 * @p: an instance
 *
 * This hook dumps the complete state of the instance to the
 * debug output.
 */
void dump(struct proto *p)
{ DUMMY; }

/**
 * dump_attrs - dump protocol-dependent attributes
 * @e: a route entry
 *
 * This hook dumps all attributes in the &rte which belong to this
 * protocol to the debug output.
 */
void dump_attrs(rte *e)
{ DUMMY; }

/**
 * start - request instance startup
 * @p: protocol instance
 *
 * The start() hook is called by the core when it wishes to start
 * the instance. Multitable protocols should lock their tables here.
 *
 * Result: new protocol state
 */
int start(struct proto *p)
{ DUMMY; }

/**
 * shutdown - request instance shutdown
 * @p: protocol instance
 *
 * The stop() hook is called by the core when it wishes to shut
 * the instance down for some reason.
 *
 * Returns: new protocol state
 */
int shutdown(struct proto *p)
{ DUMMY; }

/**
 * cleanup - request instance cleanup
 * @p: protocol instance
 *
 * The cleanup() hook is called by the core when the protocol became
 * hungry/down, i.e. all protocol ahooks and routes are flushed.
 * Multitable protocols should unlock their tables here.
 */
void cleanup(struct proto *p)
{ DUMMY; }

/**
 * get_status - get instance status
 * @p: protocol instance
 * @buf: buffer to be filled with the status string
 *
 * This hook is called by the core if it wishes to obtain an brief one-line user friendly
 * representation of the status of the instance to be printed by the <cf/show protocols/
 * command.
 */
void get_status(struct proto *p, byte *buf)
{ DUMMY; }

/**
 * get_route_info - get route information
 * @e: a route entry
 * @buf: buffer to be filled with the resulting string
 * @attrs: extended attributes of the route
 *
 * This hook is called to fill the buffer @buf with a brief user friendly
 * representation of metrics of a route belonging to this protocol.
 */
void get_route_info(rte *e, byte *buf, ea_list *attrs)
{ DUMMY; }

/**
 * get_attr - get attribute information
 * @a: an extended attribute
 * @buf: buffer to be filled with attribute information
 *
 * The get_attr() hook is called by the core to obtain a user friendly
 * representation of an extended route attribute. It can either leave
 * the whole conversion to the core (by returning %GA_UNKNOWN), fill
 * in only attribute name (and let the core format the attribute value
 * automatically according to the type field; by returning %GA_NAME)
 * or doing the whole conversion (used in case the value requires extra
 * care; return %GA_FULL).
 */
int get_attr(eattr *a, byte *buf, int buflen)
{ DUMMY; }

/**
 * if_notify - notify instance about interface changes
 * @p: protocol instance
 * @flags: interface change flags
 * @i: the interface in question
 *
 * This hook is called whenever any network interface changes its status.
 * The change is described by a combination of status bits (%IF_CHANGE_xxx)
 * in the @flags parameter.
 */
void if_notify(struct proto *p, unsigned flags, struct iface *i)
{ DUMMY; }

/**
 * ifa_notify - notify instance about interface address changes
 * @p: protocol instance
 * @flags: address change flags
 * @a: the interface address
 *
 * This hook is called to notify the protocol instance about an interface
 * acquiring or losing one of its addresses. The change is described by
 * a combination of status bits (%IF_CHANGE_xxx) in the @flags parameter.
 */
void ifa_notify(struct proto *p, unsigned flags, struct ifa *a)
{ DUMMY; }

/**
 * rt_notify - notify instance about routing table change
 * @p: protocol instance
 * @table: a routing table 
 * @net: a network entry
 * @new: new route for the network
 * @old: old route for the network
 * @attrs: extended attributes associated with the @new entry
 *
 * The rt_notify() hook is called to inform the protocol instance about
 * changes in the connected routing table @table, that is a route @old
 * belonging to network @net being replaced by a new route @new with
 * extended attributes @attrs. Either @new or @old or both can be %NULL
 * if the corresponding route doesn't exist.
 *
 * If the type of route announcement is RA_OPTIMAL, it is an
 * announcement of optimal route change, @new stores the new optimal
 * route and @old stores the old optimal route.
 *
 * If the type of route announcement is RA_ANY, it is an announcement
 * of any route change, @new stores the new route and @old stores the
 * old route from the same protocol.
 *
 * @p->accept_ra_types specifies which kind of route announcements
 * protocol wants to receive.
 */
void rt_notify(struct proto *p, net *net, rte *new, rte *old, ea_list *attrs)
{ DUMMY; }

/**
 * neigh_notify - notify instance about neighbor status change
 * @neigh: a neighbor cache entry
 *
 * The neigh_notify() hook is called by the neighbor cache whenever
 * a neighbor changes its state, that is it gets disconnected or a
 * sticky neighbor gets connected.
 */
void neigh_notify(neighbor *neigh)
{ DUMMY; }

/**
 * make_tmp_attrs - convert embedded attributes to temporary ones
 * @e: route entry
 * @pool: linear pool to allocate attribute memory in
 *
 * This hook is called by the routing table functions if they need
 * to convert the protocol attributes embedded directly in the &rte
 * to temporary extended attributes in order to distribute them
 * to other protocols or to filters. make_tmp_attrs() creates
 * an &ea_list in the linear pool @pool, fills it with values of the
 * temporary attributes and returns a pointer to it.
 */
ea_list *make_tmp_attrs(rte *e, struct linpool *pool)
{ DUMMY; }

/**
 * store_tmp_attrs - convert temporary attributes to embedded ones
 * @e: route entry
 * @attrs: temporary attributes to be converted
 *
 * This hook is an exact opposite of make_tmp_attrs() -- it takes
 * a list of extended attributes and converts them to attributes
 * embedded in the &rte corresponding to this protocol.
 *
 * You must be prepared for any of the attributes being missing
 * from the list and use default values instead.
 */
void store_tmp_attrs(rte *e, ea_list *attrs)
{ DUMMY; }

/**
 * import_control - pre-filtering decisions on route import
 * @p: protocol instance the route is going to be imported to
 * @e: the route in question
 * @attrs: extended attributes of the route
 * @pool: linear pool for allocation of all temporary data
 *
 * The import_control() hook is called as the first step of a exporting
 * a route from a routing table to the protocol instance. It can modify
 * route attributes and force acceptance or rejection of the route regardless
 * of user-specified filters. See rte_announce() for a complete description
 * of the route distribution process.
 *
 * The standard use of this hook is to reject routes having originated
 * from the same instance and to set default values of the protocol's metrics.
 *
 * Result: 1 if the route has to be accepted, -1 if rejected and 0 if it
 * should be passed to the filters.
 */
int import_control(struct proto *p, rte **e, ea_list **attrs, struct linpool *pool)
{ DUMMY; }

/**
 * rte_recalculate - prepare routes for comparison
 * @table: a routing table 
 * @net: a network entry
 * @new: new route for the network
 * @old: old route for the network
 * @old_best: old best route for the network (may be NULL)
 *
 * This hook is called when a route change (from @old to @new for a
 * @net entry) is propagated to a @table. It may be used to prepare
 * routes for comparison by rte_better() in the best route
 * selection. @new may or may not be in @net->routes list,
 * @old is not there.
 *
 * Result: 1 if the ordering implied by rte_better() changes enough
 * that full best route calculation have to be done, 0 otherwise.
 */
int rte_recalculate(struct rtable *table, struct network *net, struct rte *new, struct rte *old, struct rte *old_best)
{ DUMMY; }

/**
 * rte_better - compare metrics of two routes
 * @new: the new route
 * @old: the original route
 *
 * This hook gets called when the routing table contains two routes
 * for the same network which have originated from different instances
 * of a single protocol and it wants to select which one is preferred
 * over the other one. Protocols usually decide according to route metrics.
 *
 * Result: 1 if @new is better (more preferred) than @old, 0 otherwise.
 */
int rte_better(rte *new, rte *old)
{ DUMMY; }

/**
 * rte_same - compare two routes
 * @e1: route
 * @e2: route
 *
 * The rte_same() hook tests whether the routes @e1 and @e2 belonging
 * to the same protocol instance have identical contents. Contents of
 * &rta, all the extended attributes and &rte preference are checked
 * by the core code, no need to take care of them here.
 *
 * Result: 1 if @e1 is identical to @e2, 0 otherwise.
 */
int rte_same(rte *e1, rte *e2)
{ DUMMY; }

/**
 * rte_insert - notify instance about route insertion
 * @n: network
 * @e: route
 *
 * This hook is called whenever a &rte belonging to the instance
 * is accepted for insertion to a routing table.
 *
 * Please avoid using this function in new protocols.
 */
void rte_insert(net *n, rte *e)
{ DUMMY; }

/**
 * rte_remove - notify instance about route removal
 * @n: network
 * @e: route
 *
 * This hook is called whenever a &rte belonging to the instance
 * is removed from a routing table.
 *
 * Please avoid using this function in new protocols.
 */
void rte_remove(net *n, rte *e)
{ DUMMY; }
