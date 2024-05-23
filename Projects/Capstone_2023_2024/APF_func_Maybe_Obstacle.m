function [phi_tot,phi_tot_grad,u, state_desired] = APF_func_Maybe_Obstacle(state_red,state_black,state_blue,shape,ka,kr,Q_a,Q_b,P_b,r_hold,Binv,K_a,psi,sigma,r_off,Nmat,IsBlue)


state_desired = [state_black(1)-r_hold*sin(state_black(3))+r_off*cos(state_black(3))
    state_black(2)+r_off*sin(state_black(3))+r_hold*cos(state_black(3))
    state_black(3)+pi
    state_black(4)-r_hold*sin(state_black(3))*state_black(6)+r_off*cos(state_black(3))*state_black(6)
    state_black(5)+r_hold*cos(state_black(3))*state_black(6)+r_off*sin(state_black(3))*state_black(6)
    state_black(6)];

if IsBlue == 0

    p_f = state_desired(1:3); % desired state
    p_c = state_red(1:3); % current state of chaser
    p_c_dot = state_red(4:6); %state derivative of chaser
    p_b = state_black(1:3); % current state of target
    p_o = state_blue(1:3); % current state of the obstacle

    r_cf = p_c - p_f; %relative difference between chaser and desired
    r_ct = [state_red(1); state_red(2); state_black(3)] - p_b; %relative difference between chaser and target
    r_co = [state_red(1); state_red(2); state_blue(3)] - p_o; %relative difference between chaser and obstacle

    [~,d] = Cardioid(state_red,state_black,shape);
    r_cb = r_ct - d.*(r_ct)/norm(r_ct,2);


    phi_a = ka/2*r_cf'*Q_a*r_cf; % Attractive potential on target
    phi_b = kr*exp(-r_cb'*P_b*r_cb); %Repulsive potential on target keep out zones
    phi_r = psi*exp(-r_co'*Nmat*r_co./sigma); %Repulsive potential on obstacle


    phi_tot = phi_a + phi_b + phi_r; %Total potential
    phi_tot_grad = ka*Q_a*r_cf - 2*kr*exp(-r_cb'*P_b*r_cb).*(P_b*r_cb) - (2*psi/sigma*exp(-r_co'*Nmat*r_co./sigma)).*(Nmat*r_co); %Potential gradient
    u = -Binv*K_a*(p_c_dot+phi_tot_grad);


else

    p_f = state_desired(1:3); % desired state
    p_c = state_red(1:3); % current state of chaser
    p_c_dot = state_red(4:6); %state derivative of chaser
    p_b = state_black(1:3); % current state of target

    r_cf = p_c - p_f; %relative difference between chaser and desired
    r_ct = [state_red(1); state_red(2); state_black(3)] - p_b; %relative difference between chaser and target

    [~,d] = Cardioid(state_red,state_black,shape);
    r_cb = r_ct - d.*(r_ct)/norm(r_ct,2);

    phi_a = ka/2*r_cf'*Q_a*r_cf; % Attractive potential on target
    phi_b = kr*exp(-r_cb'*P_b*r_cb); %Repulsive potential on target keep out zones
  
    phi_tot = phi_a + phi_b; %Total potential
    phi_tot_grad = ka*Q_a*r_cf - 2*kr*exp(-r_cb'*P_b*r_cb).*(P_b*r_cb); %Potential gradient
    u = -Binv*K_a*(p_c_dot+phi_tot_grad);


end


end