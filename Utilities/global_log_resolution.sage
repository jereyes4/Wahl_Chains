
def LocalTree(ring, curves: dict, point, curve_index : int, no_base_curves : int):

    x,y = ring.gens()

    # Extend field to move point to (0,0).
    field = ring.base_ring()
    minimal_y = point.elimination_ideal(x).gen(0)
    y_multiplicity = minimal_y.degree()
    if y_multiplicity > 1:
        t = PolynomialRing(field,'t').gen()

        field_y_mid = field.extension(minimal_y(0,t),'w')
        ypos = field_y_mid.gen()
        field_y_solved = field_y_mid.absolute_field('w')
        _,F_y_in = field_y_solved.structure()
        ypos = F_y_in(ypos)
    else:
        field_y_solved = field
        ypos = -minimal_y[0,0]/minimal_y[0,1]
        F_y_in = lambda a : a

    uniRing = PolynomialRing(field_y_solved,'t')
    t = uniRing.gen()

    exceptionals_here = []

    exceptional_parents = [i for i in curves.keys() if i >= no_base_curves]

    sol_x = gcd([sum([F_y_in(coeff)*t**(exponent[0])*ypos**(exponent[1]) for exponent, coeff in f.dict().items()]) for f in point.gens()])

    for factor_pair in factor(sol_x):
        x_factor = uniRing(factor_pair[0])
        if x_factor.is_constant():
            continue

        x_multiplicity = x_factor.degree()
        if x_multiplicity > 1:
            field_x_mid = field_y_solved.extension(x_factor,'v')
            xpos = field_x_mid.gen()
            field_x_solved = field_x_mid.absolute_field('v')
            _, F_x_in = field_x_solved.structure()
            xpos = F_x_in(xpos)
        else:
            field_x_solved = field_y_solved
            xpos = -x_factor[0]/x_factor[1]
            F_x_in = lambda a : a

        newRing = PolynomialRing(field_x_solved,[x,y])
        x,y = newRing.gens()
        trans_curves = {i : sum([F_x_in(F_y_in(coeff))*(x+xpos)**(exponent[0])*(y+F_x_in(ypos))**(exponent[1]) for exponent, coeff in curve.dict().items()]) for i, curve in curves.items() }

        exceptional_index = curve_index
        curve_index += 1
        exceptionals_here.append([exceptional_index, y_multiplicity*x_multiplicity, list(exceptional_parents)])

        # Blowup (x,y) -> (x*y,y) for points [x:1]
        newCurves = {i : curve(x*y,y)//(y**(min([a[0] + a[1] for a in curve.exponents()]))) for i,curve in trans_curves.items()}

        newPoints_x = set([u for f in newCurves.values() for u,_ in factor(f(x,0))])

        newCurves[exceptional_index] = y


        for newPoint_x in newPoints_x:
            newPoint_x = newRing(newPoint_x)
            if newPoint_x.is_constant():
                continue
            Curves_through_point = {i : curve for i,curve in newCurves.items() if newPoint_x.divides(curve(x,0))}

            if len(Curves_through_point) == 2:
                non_ex_id = -1
                skip = True
                for i,curve in Curves_through_point.items():
                    if curve != y:
                        if (newPoint_x**2).divides(curve(x,0)):
                            skip = False
                        else:
                            non_ex_id = i
                        break
                if skip:
                    if non_ex_id < no_base_curves:
                        exceptionals_here[-1].append([non_ex_id,newPoint_x.degree()])
                    continue

            newPoint = ideal(newPoint_x,y)
            
            exceptionals_child, curve_index = LocalTree(newRing,Curves_through_point,newPoint,curve_index, no_base_curves)

            exceptionals_here[-1] += exceptionals_child

        # Blowup (x,y) -> (x,x*y) for point [1:0]
        newCurves = {i : curve(x,x*y)//(x**(min([a[0] + a[1] for a in curve.exponents()]))) for i,curve in trans_curves.items()}
        newCurves[exceptional_index] = x

        Curves_through_point = {i : curve for i,curve in newCurves.items() if curve(0,0) == 0}
        skip = False
        if len(Curves_through_point) == 1:
            skip = True
        elif len(Curves_through_point) == 2:
            non_ex_id = -1
            skip = True
            for i,curve in Curves_through_point.items():
                if curve != x:
                    if (y**2).divides(curve(0,y)):
                        skip = False
                    else:
                        non_ex_id = i
                    break
            if skip and non_ex_id < no_base_curves:
                exceptionals_here[-1].append([non_ex_id,1])
                
        if not skip:
            newPoint = ideal(x,y)
            
            exceptionals_child, curve_index = LocalTree(newRing,Curves_through_point,newPoint,curve_index, no_base_curves)

            exceptionals_here[-1] += exceptionals_child
    
    return exceptionals_here, curve_index


def global_log_resolution(curves: list, include_ideal=False):
    if not curves:
        raise TypeError("List is empty.")
    ring = curves[0].parent()
    field = ring.base_ring()
    x,y,z = ring.gens()
    for curve in curves:
        if curve.parent() != ring:
            raise TypeError("All curves must belong to the same ring.")
        if not curve.is_homogeneous():
            raise TypeError("{} is not homogeneous".format(curve))
        if not curve.is_prime():
            raise TypeError("{} is not prime".format(curve))
        if curve.is_constant():
            raise TypeError("{} is constant".format(curve))

    seenPoints = set()
    for i in range(len(curves)):
        for j in range(i+1,len(curves)):
            if curves[i].divides(curves[j]):
                raise TypeError("{} defines the same curves as {}".format(curves[i],curves[j]))
            points = ideal(curves[i],curves[j]).associated_primes()
            for point in points:
                if point == ring.irrelevant_ideal():
                    continue
                seenPoints.add(point)
        points = ideal(curves[i].derivative(y),curves[i].derivative(x),curves[i].derivative(z)).associated_primes()
        for point in points:
            if point == ring.irrelevant_ideal():
                continue
            seenPoints.add(point)

    
    forest = []
    curve_index = len(curves)

    for point in seenPoints:
        dehomRing = PolynomialRing(field,[x,y])
        dhx, dhy = dehomRing.gens()
        if z not in point:
            Curves_through_point = {i : curves[i](dhx,dhy,1) for i in range(len(curves)) if curves[i] in point}
            dehomPoint = ideal([f(dhx,dhy,1) for f in point.gens()])
            result_tree, curve_index = LocalTree(dehomRing,Curves_through_point,dehomPoint,curve_index, len(curves))
            if include_ideal:
                forest += [point,result_tree]
            else:
                forest += result_tree
        elif y not in point:
            Curves_through_point = {i : curves[i](dhx,1,dhy) for i in range(len(curves)) if curves[i] in point}
            dehomPoint = ideal([f(dhx,1,dhy) for f in point.gens()])
            result_tree, curve_index = LocalTree(dehomRing,Curves_through_point,dehomPoint,curve_index, len(curves))
            if include_ideal:
                forest += [point,result_tree]
            else:
                forest += result_tree
        else:
            Curves_through_point = {i : curves[i](1,dhx,dhy) for i in range(len(curves)) if curves[i] in point}
            dehomPoint = ideal([f(1,dhx,dhy) for f in point.gens()])
            result_tree, curve_index = LocalTree(dehomRing,Curves_through_point,dehomPoint,curve_index, len(curves))
            if include_ideal:
                forest += [point,result_tree]
            else:
                forest += result_tree
    return forest